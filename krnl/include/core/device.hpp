#pragma once
#include <webgpu/webgpu_cpp.h>

namespace krnl
{

    class Device
    {
    public:
        explicit Device(wgpu::Instance instance);
		~Device() = default;
        wgpu::Device get() const { return m_device; }
		wgpu::Queue getQueue() const { return m_device.GetQueue(); }

    private:
        wgpu::Device m_device;
		wgpu::Queue m_queue;
    };

} // namespace krnl
