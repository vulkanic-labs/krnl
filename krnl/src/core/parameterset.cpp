#include "core/parameterset.hpp"
#include "core/log.h"
#include <cassert>
#include <vector>

namespace krnl {

    ParameterSet::ParameterSet(Device& device, const std::vector<Entry>& entries)
        : m_device(device), m_entries(entries)
    {
        buildLayout();
        buildBindGroup();
    }

    void ParameterSet::update() {
        buildBindGroup();
    }

    void ParameterSet::buildLayout() {
        std::vector<wgpu::BindGroupLayoutEntry> layoutEntries;
        layoutEntries.reserve(m_entries.size());

        for (uint32_t i = 0; i < m_entries.size(); ++i) {
            const Entry& e = m_entries[i];
            assert(&e.buffer != nullptr && "ParameterSet entry buffer must not be null");

            wgpu::BindGroupLayoutEntry be{};
            be.binding = i;
            be.visibility = wgpu::ShaderStage::Compute;

            // Use explicit bindingType provided by caller
            be.buffer.type = static_cast<wgpu::BufferBindingType>(e.bindingType);
            be.buffer.hasDynamicOffset = false;

            // If uniform, set minBindingSize aligned to 256 for portability
            if (e.bindingType == krnl::BufferBindingType::Uniform) {
                //be.buffer.minBindingSize = e.buffer->sizeAlignedToUniform();
            }
            else {
                be.buffer.minBindingSize = 0;
            }

            layoutEntries.push_back(be);
        }

        wgpu::BindGroupLayoutDescriptor desc{};
        desc.entryCount = static_cast<uint32_t>(layoutEntries.size());
        desc.entries = layoutEntries.data();

        m_bindGroupLayout = m_device.GetNative().CreateBindGroupLayout(&desc);
    }   

    void ParameterSet::buildBindGroup() {
        std::vector<wgpu::BindGroupEntry> entries;
        entries.reserve(m_entries.size());

        for (uint32_t i = 0; i < m_entries.size(); ++i) {
            const Entry& e = m_entries[i];
            assert(&e.buffer != nullptr);

            wgpu::BindGroupEntry ent{};
            ent.binding = i;
            ent.buffer = e.buffer.GetNative();
            ent.offset = 0;
            ent.size = static_cast<uint64_t>(e.buffer.GetSize());
            entries.push_back(ent);
        }

        wgpu::BindGroupDescriptor desc{};
        desc.layout = m_bindGroupLayout;
        desc.entryCount = static_cast<uint32_t>(entries.size());
        desc.entries = entries.data();

        m_bindGroup = m_device.GetNative().CreateBindGroup(&desc);
    }

} // namespace krnl
