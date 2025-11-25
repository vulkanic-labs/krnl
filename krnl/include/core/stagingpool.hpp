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
     * - For upload (MapWrite | CopySrc) we create it with mappedAtCreation = true
     *   so callers can write to ->mappedPtr immediately.
     *
     * NOTE: mappedPtr is only valid until you call Unmap() on the buffer.
     * Pool will create a replacement buffer after you submit the upload.
     */
    struct StagingHandle {
        wgpu::Buffer buffer;
        void* mappedPtr = nullptr;     // valid only when buffer is mapped
        size_t size = 0;
        bool forWrite = true;
        // inUse is a hint to the pool (used->will be submitted).
        bool inUse = false;
    };
    using StagingHandlePtr = std::shared_ptr<StagingHandle>;

    /**
     * PersistentStagingPool
     *
     * Usage pattern (uploads):
     *  - call allocate(size) -> returns a StagingHandlePtr with mappedPtr ready to write
     *  - memcpy into handle->mappedPtr
     *  - call submitUpload(handle, dstBuffer, bytes, dstOffset, queue)
     *      - this will Unmap the staging buffer, record and submit a CopyBufferToBuffer,
     *        and immediately create a replacement mapped staging buffer and return it in place.
     *
     * The pool keeps the submitted staging handle alive until the function returns (shared_ptr).
     * If you want the pool to free memory only after GPU finishes, you must add fence-based completion detection.
     */
    class PersistentStagingPool {
    public:
        struct Config {
            // Maximum number of staging buffers to keep alive in the ready pool
            size_t maxPoolSize = 4;
            // If true, the pool uses locking to be thread-safe (slower). Default true.
            bool threadSafe = true;
            // Label prefix for created staging buffers
            const char* labelPrefix = "krnl_staging";
        };

        PersistentStagingPool(wgpu::Device device, const Config& cfg = {});
        ~PersistentStagingPool();

        // Acquire a staging buffer with at least 'size' bytes. Provided buffer is mapped and ready to write.
        StagingHandlePtr allocate(size_t size);

        // Submit an upload: unmap the staging, copy staging->dstBuffer, submit on queue,
        // and replenish a new mapped staging buffer for future use.
        // - dstOffset is the offset into dstBuffer where data should be copied.
        // - After calling submitUpload, the staging handle you passed is "consumed" (it will be unmapped).
        // - The pool creates a new mapped staging buffer to replace it. The function returns a new replacement
        //   staging handle that you may use for the next upload (or nullptr if pool full).
        StagingHandlePtr submitUpload(
            StagingHandlePtr staging,
            wgpu::Buffer dstBuffer,
            size_t bytes,
            size_t dstOffset,
            wgpu::Queue queue
        );

        // Optionally free all idle staging buffers
        void purge();

    private:
        StagingHandlePtr createStaging(size_t size);

        wgpu::Device m_device;
        Config m_cfg;

        // ready (mapped) staging buffers that can be reused
        std::vector<StagingHandlePtr> m_ready;

        // in-flight staging buffers we keep alive (submitted but not otherwise tracked)
        std::vector<StagingHandlePtr> m_inflight;

        // small mutex for thread-safety if requested
        std::mutex m_mutex;
    };

} // namespace krnl
