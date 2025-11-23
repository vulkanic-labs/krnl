#pragma once
#include "core/device.hpp"
#include "core/buffer.hpp"
#include <vector>

namespace krnl
{

    class Tensor
    {
    public:
        // Tensor() = default;
        // Tensor(Device device, const std::vector<float>& data);

        // size_t size() const { return dataSize; }
        // wgpu::Buffer get() const { return buffer.get(); }

    private:
        Buffer buffer;
        size_t dataSize = 0;
    };

} // namespace krnl
