#pragma once

#include <webgpu/webgpu_cpp.h>
#include "core/device.hpp"
#include "core/buffer.hpp"

namespace krnl {

    class CommandList {
    public:
        CommandList(Device& device)
            : m_device(device), m_encoder(device.GetNative().CreateCommandEncoder()) {
        }

        ~CommandList() = default;

        // No copying
        CommandList(const CommandList&) = delete;
        CommandList& operator=(const CommandList&) = delete;

        // Moves allowed
        CommandList(CommandList&&) = default;

        // ---- Compute pass ----
        void BeginComputePass();
        void EndComputePass();

        // ---- Copy operations ----
        void CopyBufferToBuffer(const Buffer& src, const Buffer& dst, size_t size);

        // ---- Finalize ----
        wgpu::CommandBuffer Finish();
        void Submit();

		const wgpu::CommandEncoder& GetEncoder() const { return m_encoder; }
		const wgpu::ComputePassEncoder& GetComputePass() const { return m_computePass; }

    private:
        Device& m_device;
        wgpu::CommandEncoder m_encoder;
        wgpu::ComputePassEncoder m_computePass;
    };

} // namespace krnl
