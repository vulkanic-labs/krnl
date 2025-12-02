#pragma once
#include <webgpu/webgpu_cpp.h>
#include "core/instance.hpp"

namespace krnl
{

    class Device
    {
    public:
        explicit Device(const Instance& instance);

        Device(Device&&) = default;
        Device& operator=(Device&&) = default;

        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;


		~Device() = default;
        const wgpu::Device GetNative() const { return m_Device; }
		const wgpu::Queue getQueue() const { return m_Queue; }

        bool IsValid() const { return m_Device != nullptr; }

    private:
        Device() = default;
        wgpu::Device m_Device;
		wgpu::Queue m_Queue;
    };

} // namespace krnl
