#pragma once
#include <webgpu/webgpu_cpp.h>
#include <vector>

namespace krnl
{

    class Buffer
    {
    public:
        // Buffer() = default;
        // Buffer(wgpu::Device device, size_t size, wgpu::BufferUsage usage);
        // void write(const void* data, size_t size);
        // wgpu::Buffer get() const { return buffer; }

    private:
        wgpu::Device device;
        wgpu::Buffer buffer;
        size_t bufferSize = 0;
    };

} // namespace krnl
