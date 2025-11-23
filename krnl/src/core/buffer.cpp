#include "core/buffer.hpp"

namespace krnl
{

    // Buffer::Buffer(wgpu::Device device, size_t size, wgpu::BufferUsage usage)
    //     : device(device), bufferSize(size)
    // {
    //     // WGPUBufferDescriptor desc = {};
    //     // desc.size = size;
    //     // desc.usage = usage | WGPUBufferUsage_CopyDst | WGPUBufferUsage_CopySrc;
    //     // buffer = device.CreateBuffer(&desc);
    // }

    // void Buffer::write(const void *data, size_t size)
    // {
    //     // device.GetQueue().WriteBuffer(buffer, 0, data, size);
    // }

} // namespace krnl
