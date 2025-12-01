#pragma once
#include <webgpu/webgpu_cpp.h>
#include <memory>
#include <vector>
#include <mutex>
#include <functional>
#include <cstddef>

namespace krnl {

    /**
     * A single staging buffer owned by the pool.
     * - For uploads (MapWrite | CopySrc) we create it with mappedAtCreation = true
     *   so callers can write to ->mappedPtr immediately.
     */
    struct StagingHandle {
        wgpu::Buffer buffer;
        void* mappedPtr = nullptr;     // valid only when buffer is mapped (forWrite==true)
        size_t size = 0;
        bool forWrite = true;
        bool inUse = false;
    };
    using StagingHandlePtr = std::shared_ptr<StagingHandle>;

    class PersistentStagingPool {
    public:
        struct Config {
            size_t maxPoolSize = 4;
            bool threadSafe = true;
            const char* labelPrefix = "krnl_staging";
        };

        PersistentStagingPool(wgpu::Device device, const Config& cfg = {});
        ~PersistentStagingPool();

        // Acquire a staging buffer with at least 'size' bytes. The returned handle is mapped and ready to write
        // if forWrite is true.
        StagingHandlePtr allocate(size_t size);

        // Submit an upload: unmap the staging (if mapped), copy staging -> dstBuffer, submit on queue,
        // and create a replacement mapped staging buffer for future use if pool isn't saturated.
        // Returns a replacement staging handle (may be null if none created).
        StagingHandlePtr submitUpload(
            StagingHandlePtr staging,
            wgpu::Buffer dstBuffer,
            size_t bytes,
            size_t dstOffset,
            wgpu::Queue queue
        );

        // Acquire a staging buffer intended for readback (MapRead). This will produce a staging
        // buffer that includes MapRead | CopyDst | CopySrc for safety.
        StagingHandlePtr allocateForReadback(size_t size);

        // Copy from a source GPU buffer into a readback staging buffer and map it async.
        // This helper creates a staging (MapRead|CopyDst|CopySrc), issues the copy, submits, maps it,
        // and calls callback with the mapped pointer when ready.
        void readbackInto(
            wgpu::Buffer src,
            size_t bytes,
            size_t srcOffset,
            wgpu::Queue queue,
            std::function<void(const void* data, size_t size)> cb
        );

        void purge();

    private:
        StagingHandlePtr createStaging(size_t size);
        StagingHandlePtr createReadbackStaging(size_t size);

        wgpu::Device m_device;
        Config m_cfg;
        std::vector<StagingHandlePtr> m_ready;
        std::vector<StagingHandlePtr> m_inflight;
        std::mutex m_mutex;
    };

} // namespace krnl
