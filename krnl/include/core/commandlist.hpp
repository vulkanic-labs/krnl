#pragma once

#include <webgpu/webgpu_cpp.h>
#include "core/device.hpp"
#include "core/buffer.hpp"

namespace krnl {

    class CommandList {
    public:
        CommandList(const Device& device)
            : m_Device(device), m_Encoder(device.GetNative().CreateCommandEncoder()) {
        }

        ~CommandList() = default;

        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;
        CommandList(CommandList&&) = default;

        void BeginComputePass();
        void EndComputePass();

        void CopyBufferToBuffer(const Buffer& src, const Buffer& dst, size_t size);

        wgpu::CommandBuffer Finish();
        void Submit();

		const wgpu::CommandEncoder& GetEncoder() const { return m_Encoder; }
		const wgpu::ComputePassEncoder& GetComputePass() const { return m_ComputePass; }

    private:
        const Device& m_Device;
        wgpu::CommandEncoder m_Encoder;
        wgpu::ComputePassEncoder m_ComputePass;
    };

} // namespace krnl
