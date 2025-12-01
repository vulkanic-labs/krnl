#pragma once
#include <webgpu/webgpu_cpp.h>
#include "core/instance.hpp"

namespace krnl
{

    class Device
    {
    public:
        Device() = default;
        Device(const Instance& instance);
		~Device() = default;
        wgpu::Device GetNative() const { return m_device; }
		wgpu::Queue getQueue() const { return m_device.GetQueue(); }

    private:
        wgpu::Device m_device;
		wgpu::Queue m_queue;
    };

} // namespace krnl
