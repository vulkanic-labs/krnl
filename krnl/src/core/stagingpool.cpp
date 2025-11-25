#include "core/staging_pool.hpp"
#include "core/log.h"
#include <algorithm>
#include <cstring>

namespace krnl {

    PersistentStagingPool::PersistentStagingPool(wgpu::Device device, const Config& cfg)
        : m_device(device), m_cfg(cfg)
    {
        // reserve initial capacity
        m_ready.reserve(m_cfg.maxPoolSize);
        m_inflight.reserve(m_cfg.maxPoolSize * 2);
    }

    PersistentStagingPool::~PersistentStagingPool() {
        // Ensure all buffers are destroyed / unmapped (Dawn will handle destruction on object destruction).
        purge();
    }

    StagingHandlePtr PersistentStagingPool::createStaging(size_t size) {
        // Round up size to 256 or 4K to reduce fragmentation
        constexpr size_t ALIGN = 256;
        size = ((size + ALIGN - 1) / ALIGN) * ALIGN;

        wgpu::BufferDescriptor desc{};
        desc.size = size;
        // MapWrite + CopySrc for upload staging.
        desc.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
        desc.mappedAtCreation = true;

        // make label
        std::string label = std::string(m_cfg.labelPrefix) + "_upload";
        desc.label = label.c_str();

        wgpu::Buffer staging = m_device.CreateBuffer(&desc);

        // mappedAtCreation => GetMappedRange returns pointer
        void* mapped = staging.GetMappedRange();
        if (!mapped) {
            KRNL_WARN("PersistentStagingPool::createStaging mappedAtCreation returned nullptr (size=%zu)", size);
        }

        auto h = std::make_shared<StagingHandle>();
        h->buffer = staging;
        h->mappedPtr = mapped;
        h->size = size;
        h->forWrite = true;
        h->inUse = false;
        return h;
    }

    StagingHandlePtr PersistentStagingPool::allocate(size_t size) {
        if (m_cfg.threadSafe) m_mutex.lock();

        // find a ready buffer >= requested size
        auto it = std::find_if(m_ready.begin(), m_ready.end(), [size](const StagingHandlePtr& h) { return h->size >= size && !h->inUse; });
        if (it != m_ready.end()) {
            StagingHandlePtr ret = *it;
            // remove from ready pool
            m_ready.erase(it);
            ret->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return ret;
        }

        // if pool not full, make a new staging
        if (m_ready.size() + m_inflight.size() < m_cfg.maxPoolSize) {
            StagingHandlePtr newStaging = createStaging(size);
            newStaging->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return newStaging;
        }

        // Pool is full: try to re-use smallest ready or fall back to making a new one anyway
        if (!m_ready.empty()) {
            // get the smallest adequate (or smallest available) and use it
            auto smallestIt = std::min_element(m_ready.begin(), m_ready.end(),
                [](const StagingHandlePtr& a, const StagingHandlePtr& b) { return a->size < b->size; });
            StagingHandlePtr ret = *smallestIt;
            m_ready.erase(smallestIt);
            ret->inUse = true;
            if (m_cfg.threadSafe) m_mutex.unlock();
            return ret;
        }

        // Last resort: create new staging even if over pool size
        StagingHandlePtr newStaging = createStaging(size);
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
            KRNL_ERROR("submitUpload: bytes (%zu) > staging size (%zu)", bytes, staging->size);
            return nullptr;
        }

        if (m_cfg.threadSafe) m_mutex.lock();

        // Unmap the staging (it was mappedAtCreation). After Unmap it cannot be written until remapped.
        staging->buffer.Unmap();

        // Create encoder & copy
        wgpu::CommandEncoderDescriptor encDesc{};
        wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder(&encDesc);
        encoder.CopyBufferToBuffer(staging->buffer, 0, dstBuffer, static_cast<uint64_t>(dstOffset), static_cast<uint64_t>(bytes));
        wgpu::CommandBuffer cmd = encoder.Finish();
        queue.Submit(1, &cmd);

        // keep the staging in in-flight list so it doesn't get destroyed
        m_inflight.push_back(staging);

        // Create a replacement staging buffer mappedAtCreation to keep pool ready
        StagingHandlePtr replacement = nullptr;
        if (m_ready.size() + m_inflight.size() < m_cfg.maxPoolSize + 4) {
            replacement = createStaging(bytes);
            replacement->inUse = false;
            m_ready.push_back(replacement);
        }
        else {
            // pool saturated, do not create replacement now (will be created on demand in allocate)
            KRNL_WARN("PersistentStagingPool saturated; no replacement created");
        }

        // Mark staging consumed
        staging->inUse = false;

        if (m_cfg.threadSafe) m_mutex.unlock();

        // Optionally, cleanup old in-flight items if vector grows large
        // Simple heuristic: keep in-flight limited to maxPoolSize*4; otherwise drop oldest
        if (m_inflight.size() > m_cfg.maxPoolSize * 8) {
            // Note: we don't have an explicit GPU-completion check here; removing shared_ptr may free buffer while GPU still using it on some platforms.
            // If you need stricter correctness, add fences or WaitIdle behavior per backend.
            m_inflight.erase(m_inflight.begin(), m_inflight.begin() + (m_inflight.size() / 2));
            KRNL_WARN("PersistentStagingPool trimmed in-flight list; consider increasing pool size or adding completion tracking");
        }

        return replacement;
    }

    void PersistentStagingPool::purge() {
        if (m_cfg.threadSafe) m_mutex.lock();
        m_ready.clear();
        m_inflight.clear();
        if (m_cfg.threadSafe) m_mutex.unlock();
    }

} // namespace krnl
