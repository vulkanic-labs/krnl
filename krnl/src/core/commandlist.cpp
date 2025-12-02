#include "core/commandlist.hpp"

namespace krnl {

    void CommandList::BeginComputePass() {
        m_ComputePass = m_Encoder.BeginComputePass();
    }

    void CommandList::EndComputePass() {
        m_ComputePass.End();
    }

    void CommandList::CopyBufferToBuffer(const Buffer& src, const Buffer& dst, size_t size) {
        m_Encoder.CopyBufferToBuffer(
            src.GetNative(), 0,
            dst.GetNative(), 0,
            size
        );
    }

    wgpu::CommandBuffer CommandList::Finish() {
        return m_Encoder.Finish();
    }

    void CommandList::Submit() {
        auto cmd = Finish();
        wgpu::Queue queue = m_Device.getQueue();
        queue.Submit(1, &cmd);
    }

} // namespace krnl
