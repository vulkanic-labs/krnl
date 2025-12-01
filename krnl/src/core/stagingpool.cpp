#include "core/stagingpool.hpp"
#include "core/log.h"
#include <algorithm>
#include <cstring>
#include <string>

namespace krnl {

    PersistentStagingPool::PersistentStagingPool(wgpu::Device device, const Config& cfg)
        : m_device(device), m_cfg(cfg)
    {
        m_ready.reserve(m_cfg.maxPoolSize);
        m_inflight.reserve(m_cfg.maxPoolSize * 2);
    }

    PersistentStagingPool::~PersistentStagingPool() {
        purge();
    }

    StagingHandlePtr PersistentStagingPool::createStaging(size_t size) {
        constexpr size_t ALIGN = 256;
        size = ((size + ALIGN - 1) / ALIGN) * ALIGN;

        wgpu::BufferDescriptor desc{};
        desc.size = size;
        // MapWrite + CopySrc for upload staging (mappedAtCreation=true)
        desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        desc.mappedAtCreation = true;

        std::string label = std::string(m_cfg.labelPrefix) + "_upload";
        desc.label = label.c_str();

        wgpu::Buffer b = m_device.CreateBuffer(&desc);
        void* mapped = b.GetMappedRange();

        auto h = std::make_shared<StagingHandle>();
        h->buffer = b;
        h->mappedPtr = mapped;
        h->size = size;
        h->forWrite = true;
        h->inUse = false;
        return h;
    }

    StagingHandlePtr PersistentStagingPool::createReadbackStaging(size_t size) {
        // Round to alignment
        constexpr size_t ALIGN = 256;
        size = ((size + ALIGN - 1) / ALIGN) * ALIGN;

        wgpu::BufferDescriptor desc{};
        desc.size = size;
        // IMPORTANT FIX:
        // For readback staging we MUST include CopyDst (so GPU can copy into it) AND
        // include CopySrc when you may later use it as a source. MapRead is required to map it.
        // Including CopySrc is safe and avoids usage validation errors if a copy-from-staging is ever performed.
        desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;
        desc.mappedAtCreation = false;

        std::string label = std::string(m_cfg.labelPrefix) + "_readback";
        desc.label = label.c_str();

        wgpu::Buffer b = m_device.CreateBuffer(&desc);

        auto h = std::make_shared<StagingHandle>();
        h->buffer = b;
        h->mappedPtr = nullptr;
        h->size = size;
        h->forWrite = false;
        h->inUse = false;
        return h;
    }

    StagingHandlePtr PersistentStagingPool::allocate(size_t size) {
        if (m_cfg.threadSafe) m_mutex.lock();

        auto it = std::find_if(m_ready.begin(), m_ready.end(), [size](const StagingHandlePtr& h) {
            return h->size >= size && !h->inUse;
            });

        if (it != m_ready.end()) {
            auto h = *it;
            m_ready.erase(it);
            h->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return h;
        }

        if (m_ready.size() + m_inflight.size() < m_cfg.maxPoolSize) {
            auto newStaging = createStaging(size);
            newStaging->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return newStaging;
        }

        if (!m_ready.empty()) {
            auto smallestIt = std::min_element(m_ready.begin(), m_ready.end(),
                [](const StagingHandlePtr& a, const StagingHandlePtr& b) { return a->size < b->size; });
            auto h = *smallestIt;
            m_ready.erase(smallestIt);
            h->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return h;
        }

        auto newStaging = createStaging(size);
        newStaging->inUse = true;
        if (m_cfg.threadSafe) m_mutex.unlock();
        return newStaging;
    }

    StagingHandlePtr PersistentStagingPool::submitUpload(
        StagingHandlePtr staging,
        wgpu::Buffer dstBuffer,
        size_t bytes,
        size_t dstOffset,
        wgpu::Queue queue)
    {
        if (!staging) {
            KRNL_ERROR("submitUpload called with null staging");
            return nullptr;
        }
        if (bytes > staging->size) {
            KRNL_ERROR("submitUpload: bytes (%zu) > staging size (%zu)" << bytes << staging->size);
            return nullptr;
        }

        if (m_cfg.threadSafe) m_mutex.lock();

        // Unmap the staging (was mappedAtCreation)
        staging->buffer.Unmap();

        // Encode copy: staging -> dstBuffer
        wgpu::CommandEncoderDescriptor encDesc{};
        wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder(&encDesc);
        encoder.CopyBufferToBuffer(staging->buffer, 0, dstBuffer, static_cast<uint64_t>(dstOffset), static_cast<uint64_t>(bytes));
        wgpu::CommandBuffer cmd = encoder.Finish();
        queue.Submit(1, &cmd);

        // Keep the staging alive
        m_inflight.push_back(staging);

        // Create replacement staging buffer mappedAtCreation (if pool not saturated)
        StagingHandlePtr replacement = nullptr;
        if (m_ready.size() + m_inflight.size() < m_cfg.maxPoolSize + 4) {
            replacement = createStaging(bytes);
            replacement->inUse = false;
            m_ready.push_back(replacement);
        }
        else {
            KRNL_WARN("PersistentStagingPool saturated; no replacement created");
        }

        staging->inUse = false;

        if (m_cfg.threadSafe) m_mutex.unlock();

        // Trim in-flight list occasionally (heuristic)
        if (m_inflight.size() > m_cfg.maxPoolSize * 8) {
            m_inflight.erase(m_inflight.begin(), m_inflight.begin() + (m_inflight.size() / 2));
            KRNL_WARN("PersistentStagingPool trimmed in-flight list");
        }

        return replacement;
    }

    StagingHandlePtr PersistentStagingPool::allocateForReadback(size_t size) {
        // For readback we don't return a mapped pointer; caller will submit copy and then MapAsync.
        if (m_cfg.threadSafe) m_mutex.lock();

        // Try to find an unused readback staging in ready (we store both types together).
        auto it = std::find_if(m_ready.begin(), m_ready.end(), [size](const StagingHandlePtr& h) {
            return h->size >= size && !h->inUse && !h->forWrite;
            });

        if (it != m_ready.end()) {
            auto h = *it;
            m_ready.erase(it);
            h->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return h;
        }

        // Otherwise create a new readback staging
        auto readStaging = createReadbackStaging(size);
        readStaging->inUse = true;
        if (m_cfg.threadSafe) m_mutex.unlock();
        return readStaging;
    }

    void PersistentStagingPool::readbackInto(
        wgpu::Buffer src,
        size_t bytes,
        size_t srcOffset,
        wgpu::Queue queue,
        std::function<void(const void* data, size_t size)> cb)
    {
        // Create a readback staging buffer (MapRead | CopyDst | CopySrc)
        auto staging = createReadbackStaging(bytes);

        // Encode copy: src -> staging
        wgpu::CommandEncoderDescriptor encDesc{};
        wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder(&encDesc);
        encoder.CopyBufferToBuffer(src, static_cast<uint64_t>(srcOffset), staging->buffer, 0, static_cast<uint64_t>(bytes));
        wgpu::CommandBuffer cmd = encoder.Finish();
        queue.Submit(1, &cmd);

        // Keep staging alive with shared_ptr
        auto stagingPtr = std::make_shared<wgpu::Buffer>(staging->buffer);

        stagingPtr->MapAsync(
            wgpu::MapMode::Read,
            0,
            bytes,
            wgpu::CallbackMode::WaitAnyOnly,
            [stagingPtr, cb, bytes](wgpu::MapAsyncStatus status, wgpu::StringView message) {
                if (status != wgpu::MapAsyncStatus::Success) {
                    KRNL_ERROR("PersistentStagingPool::readbackInto MapAsync failed");
                    return;
                }
                const void* mapped = stagingPtr->GetConstMappedRange(0, bytes);
                if (!mapped) {
                    KRNL_ERROR("readbackInto -> GetConstMappedRange returned null");
                    stagingPtr->Unmap();
                    return;
                }
                cb(mapped, bytes);
                stagingPtr->Unmap();
            }
        );

        // Note: we don't track stagingPtr lifetime beyond this function; cb captures stagingPtr (if needed) via lambda.
    }

    void PersistentStagingPool::purge() {
        if (m_cfg.threadSafe) m_mutex.lock();
        m_ready.clear();
        m_inflight.clear();
        if (m_cfg.threadSafe) m_mutex.unlock();
    }

} // namespace krnl
