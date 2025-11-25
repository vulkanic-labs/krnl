#include "core/parameterset.hpp"
#include <cassert>
#include <iostream>

namespace krnl {

    // -----------------------------------------------------------
    // Constructor
    // -----------------------------------------------------------
    ParameterSet::ParameterSet(
        wgpu::Device device,
        const std::vector<Entry>& entries)
        : m_device(device), m_entries(entries)
    {
        buildLayout();
        buildBindGroup();
    }

    // -----------------------------------------------------------
    // update() → rebuild bindgroup
    // -----------------------------------------------------------
    void ParameterSet::update() {
        buildBindGroup();
    }

    // -----------------------------------------------------------
    // Build BindGroupLayout based on buffer usage
    // -----------------------------------------------------------
    void ParameterSet::buildLayout() {
        std::vector<wgpu::BindGroupLayoutEntry> layoutEntries;
        layoutEntries.reserve(m_entries.size());

        uint32_t bindingIndex = 0;

        for (auto& e : m_entries) {
            auto* buf = e.buffer;
            assert(buf != nullptr);

            wgpu::BindGroupLayoutEntry entry{};
            entry.binding = bindingIndex;
            entry.visibility = e.visibility;

            // Decide whether this buffer is uniform/storage/readwrite
            if (buf->isUniform()) {
                entry.buffer.type = wgpu::BufferBindingType::Uniform;
                entry.buffer.minBindingSize = buf->sizeAlignedToUniform();
            }
            else if (buf->isStorageRW()) {
                entry.buffer.type = wgpu::BufferBindingType::Storage;
                entry.buffer.minBindingSize = buf->size();
            }
            else { // default → read-only storage
                entry.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
                entry.buffer.minBindingSize = buf->size();
            }

            layoutEntries.push_back(entry);
            bindingIndex++;
        }

        wgpu::BindGroupLayoutDescriptor desc{};
        desc.entryCount = layoutEntries.size();
        desc.entries = layoutEntries.data();

        m_bindGroupLayout = m_device.CreateBindGroupLayout(&desc);
    }

    // -----------------------------------------------------------
    // Build BindGroup
    // -----------------------------------------------------------
    void ParameterSet::buildBindGroup() {
        std::vector<wgpu::BindGroupEntry> entries;
        entries.reserve(m_entries.size());

        uint32_t bindingIndex = 0;

        for (auto& e : m_entries) {
            auto* buf = e.buffer;

            wgpu::BindGroupEntry entry{};
            entry.binding = bindingIndex;
            entry.buffer = buf->wgpuBuffer();
            entry.offset = 0;
            entry.size = buf->size();  // Dawn handles alignment

            entries.push_back(entry);
            bindingIndex++;
        }

        wgpu::BindGroupDescriptor desc{};
        desc.layout = m_bindGroupLayout;
        desc.entryCount = entries.size();
        desc.entries = entries.data();

        m_bindGroup = m_device.CreateBindGroup(&desc);
    }

} // namespace krnl
