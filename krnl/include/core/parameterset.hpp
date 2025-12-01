#pragma once
#include <vector>
#include <memory>
#include <webgpu/webgpu_cpp.h>
#include "core/buffer.hpp"
#include "core/device.hpp"

namespace krnl {

    enum class BufferBindingType : uint64_t {
        BindingNotUsed = wgpu::BufferBindingType::BindingNotUsed,
        ReadOnlyStorage = wgpu::BufferBindingType::ReadOnlyStorage,
        Storage = wgpu::BufferBindingType::Storage,
        Uniform = wgpu::BufferBindingType::Uniform,
	};

    class ParameterSet {
    public:
        struct Entry {
            krnl::Buffer& buffer;
            krnl::BufferBindingType bindingType = krnl::BufferBindingType::BindingNotUsed;
        };

        ParameterSet() = delete;
        ParameterSet(Device& device, const std::vector<Entry>& entries);

        // Rebuild bind group if buffers/entries changed
        void update();

        const wgpu::BindGroup& bindGroup() const { return m_bindGroup; }
        const wgpu::BindGroupLayout& layout() const { return m_bindGroupLayout; }

    private:
        void buildLayout();
        void buildBindGroup();

        Device m_device;
        std::vector<Entry> m_entries;

        wgpu::BindGroupLayout m_bindGroupLayout;
        wgpu::BindGroup m_bindGroup;
    };

} // namespace krnl
