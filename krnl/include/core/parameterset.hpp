#pragma once
#include <vector>
#include <memory>
#include <webgpu/webgpu_cpp.h>
#include "core/buffer.hpp"

namespace krnl {

    class ParameterSet {
    public:
        struct Entry {
            krnl::Buffer* buffer;
            wgpu::ShaderStage visibility = wgpu::ShaderStage::Compute;
        };

    public:
        ParameterSet(wgpu::Device device, const std::vector<Entry>& entries);

        void update(); // rebuild bindgroup if buffers changed

        const wgpu::BindGroup& bindGroup() const { return m_bindGroup; }
        const wgpu::BindGroupLayout& layout() const { return m_bindGroupLayout; }

    private:
        wgpu::Device m_device;
        std::vector<Entry> m_entries;

        wgpu::BindGroupLayout m_bindGroupLayout;
        wgpu::BindGroup m_bindGroup;

    private:
        void buildLayout();
        void buildBindGroup();
    };

} // namespace krnl
