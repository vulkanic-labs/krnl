#include "core/commandlist.hpp"

namespace krnl {

    void CommandList::BeginComputePass() {
        m_computePass = m_encoder.BeginComputePass();
    }

    void CommandList::EndComputePass() {
        m_computePass.End();
    }

    void CommandList::CopyBufferToBuffer(const Buffer& src, const Buffer& dst, size_t size) {
        m_encoder.CopyBufferToBuffer(
            src.GetNative(), 0,
            dst.GetNative(), 0,
            size
        );
    }

    wgpu::CommandBuffer CommandList::Finish() {
        return m_encoder.Finish();
    }

    void CommandList::Submit() {
        auto cmd = Finish();
        wgpu::Queue queue = m_device.getQueue();
        queue.Submit(1, &cmd);
    }

} // namespace krnl
