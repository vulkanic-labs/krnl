#include "core/buffer.hpp"
#include "core/device.hpp"
#include "core/log.h"
#include <cstring>
#include <cassert>

namespace krnl {

    wgpu::BufferDescriptor Buffer::makeDesc(size_t size, wgpu::BufferUsage usage, const char* label, bool mappedAtCreation) {
        wgpu::BufferDescriptor desc{};
        desc.size = size;
        desc.usage = usage;
        desc.mappedAtCreation = mappedAtCreation;
        desc.label = label ? label : nullptr;
        return desc;
    }


    Buffer::Buffer(Device& device, size_t sizeBytes, BufferUsageType usage ,std::string label , bool mappedAtCreation)
        : m_Device(device), m_size(sizeBytes), m_BufferUsageType(usage), m_Label(label), m_Buffer(nullptr) {

        wgpu::BufferDescriptor desc = makeDesc(
            sizeBytes,
            static_cast<wgpu::BufferUsage>(usage),
            m_Label.c_str(),
            mappedAtCreation
        );

        m_Buffer = m_Device.GetNative().CreateBuffer(&desc);
    }




    /* Helpers */
    //wgpu::BufferUsage Buffer::usageForType(Type t, bool forWrite) {
    //    using BU = wgpu::BufferUsage;
    //    switch (t) {
    //    case Type::Storage:
    //        // Storage buffers used for compute/read-write should allow CopySrc/CopyDst for readback & uploads
    //        return BU::Storage | BU::CopyDst | BU::CopySrc;
    //    case Type::Uniform:
    //        return BU::Uniform | BU::CopyDst;
    //    case Type::MapRead:
    //        // GPU-side buffer that you copy FROM into a MapRead staging (so must be CopySrc)
    //        return BU::Storage | BU::CopySrc;
    //    case Type::MapWrite:
    //        // staging mapped-at-creation for upload
    //        return BU::MapWrite | BU::CopyDst;
    //    case Type::Staging:
    //        // default staging for uploads (MapWrite|CopySrc)
    //        return forWrite ? (BU::MapWrite | BU::CopySrc) : (BU::MapRead | BU::CopyDst);
    //    default:
    //        return BU::None;
    //    }
    //}

  

    /* Constructors / factories */
  /*  Buffer::Buffer(wgpu::Device device, wgpu::Buffer buffer, size_t sizeBytes, Type type, wgpu::BufferUsage usage)
        : m_device(device), m_buffer(buffer), m_size(sizeBytes), m_type(type), m_usage(usage) {
    }*/

  /*  Buffer Buffer::CreateStorage(wgpu::Device device, size_t sizeBytes, const char* label ,wgpu::BufferUsage usageflag) {
		auto usage = usageForType(Type::Storage);  
        usage == wgpu::BufferUsage::None ? usage : (usage | usageflag);
        auto desc = makeDesc(sizeBytes, usage, label, false);
        wgpu::Buffer b = device.CreateBuffer(&desc);
        return Buffer(device, b, sizeBytes, Type::Storage, usage);
    }*/

    //Buffer Buffer::CreateUniform(wgpu::Device device, size_t sizeBytes, const char* label) {
    //    // Align size to 256 for safety when used in layout minBindingSize
    //    uint64_t aligned = ((sizeBytes + 255) / 256) * 256;
    //    auto usage = usageForType(Type::Uniform);
    //    auto desc = makeDesc(aligned, usage, label, false);
    //    wgpu::Buffer b = device.CreateBuffer(&desc);
    //    return Buffer(device, b, static_cast<size_t>(aligned), Type::Uniform, usage);
    //}

    //Buffer Buffer::CreateMapRead(wgpu::Device device, size_t sizeBytes, const char* label) {
    //    auto usage = usageForType(Type::MapRead);
    //    auto desc = makeDesc(sizeBytes, usage, label, false);
    //    wgpu::Buffer b = device.CreateBuffer(&desc);
    //    return Buffer(device, b, sizeBytes, Type::MapRead, usage);
    //}

    //Buffer Buffer::CreateMapWrite(wgpu::Device device, size_t sizeBytes, const char* label) {
    //    auto usage = usageForType(Type::MapWrite);
    //    auto desc = makeDesc(sizeBytes, usage, label, true); // mappedAtCreation
    //    wgpu::Buffer b = device.CreateBuffer(&desc);
    //    return Buffer(device, b, sizeBytes, Type::MapWrite, usage);
    //}

    //Buffer Buffer::CreateStaging(wgpu::Device device, size_t sizeBytes, bool forWrite, const char* label) {
    //    auto usage = usageForType(Type::Staging, forWrite);
    //    auto desc = makeDesc(sizeBytes, usage, label, forWrite); // if forWrite -> mappedAtCreation
    //    wgpu::Buffer b = device.CreateBuffer(&desc);
    //    Type t = Type::Staging;
    //    return Buffer(device, b, sizeBytes, t, usage);
    //}

    /* Label */
   /* void Buffer::setLabel(const char* label) {
        if (m_buffer) {
            m_buffer.SetLabel(label ? wgpu::StringView(label) : wgpu::StringView(nullptr));
        }
    }*/

    /* write (fast) */
    //void Buffer::write(const void* src, size_t bytes, size_t dstOffset) {
    //    assert(m_buffer);
    //    if (bytes + dstOffset > m_size) {
    //        KRNL_ERROR("Buffer::write => out of range write (requested %zu bytes at offset %zu, buffer size %zu)"<< bytes << dstOffset << m_size);
    //        return;
    //    }
    //    m_device.GetQueue().WriteBuffer(m_buffer, static_cast<uint64_t>(dstOffset), src, bytes);
    //}

    ///* writeViaStaging */
    //void Buffer::writeViaStaging(const void* src, size_t bytes, wgpu::Queue queue) {
    //    assert(m_buffer);
    //    if (bytes > m_size) {
    //        KRNL_ERROR("Buffer::writeViaStaging => bytes (%zu) > buffer size (%zu)"<< bytes <<m_size);
    //        return;
    //    }

    //    wgpu::BufferDescriptor desc = makeDesc(bytes, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, "krnl_staging_upload", true);
    //    wgpu::Buffer staging = m_device.CreateBuffer(&desc);

    //    void* mapped = staging.GetMappedRange();
    //    if (!mapped) {
    //        KRNL_ERROR("Buffer::writeViaStaging => staging.GetMappedRange() returned null");
    //        return;
    //    }
    //    std::memcpy(mapped, src, bytes);
    //    staging.Unmap();

    //    wgpu::CommandEncoderDescriptor encoderDesc{};
    //    wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder(&encoderDesc);
    //    encoder.CopyBufferToBuffer(staging, 0, m_buffer, 0, bytes);
    //    wgpu::CommandBuffer cmd = encoder.Finish();
    //    queue.Submit(1, &cmd);

    //    // keep staging alive until submission completes by retaining in local scope
    //}

    ///* readAsync */
    //void Buffer::readAsync(wgpu::Device device, wgpu::Queue queue, ReadCallback cb) {
    //    assert(m_buffer);

    //    // create staging read (MapRead | CopyDst)
    //    wgpu::BufferDescriptor desc = makeDesc(m_size, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, "krnl_staging_read", false);
    //    wgpu::Buffer staging = device.CreateBuffer(&desc);

    //    // encode copy
    //    wgpu::CommandEncoderDescriptor encoderDesc{};
    //    wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder(&encoderDesc);
    //    encoder.CopyBufferToBuffer(m_buffer, 0, staging, 0, m_size);
    //    wgpu::CommandBuffer cmd = encoder.Finish();
    //    queue.Submit(1, &cmd);

    //    auto stagingPtr = std::make_shared<wgpu::Buffer>(staging);
    //    stagingPtr->MapAsync(
    //        wgpu::MapMode::Read,
    //        0,
    //        m_size,
    //        wgpu::CallbackMode::WaitAnyOnly,
    //        [stagingPtr, cb, this](wgpu::MapAsyncStatus status, wgpu::StringView message) {
    //            if (status != wgpu::MapAsyncStatus::Success) {
    //                KRNL_ERROR("Buffer::readAsync -> MapAsync failed (status %d). msg: %.*s"<< status <<
    //                   message);
    //                return;
    //            }
    //            const void* mapped = stagingPtr->GetConstMappedRange(0, this->m_size);
    //            if (!mapped) {
    //                KRNL_ERROR("Buffer::readAsync -> GetConstMappedRange returned null");
    //                stagingPtr->Unmap();
    //                return;
    //            }
    //            cb(mapped, this->m_size);
    //            stagingPtr->Unmap();
    //        }
    //    );
    //}

    ///* Bind group helpers */
    //wgpu::BindGroupLayoutEntry Buffer::bindGroupLayoutEntry(uint32_t binding, wgpu::ShaderStage visibility) const {
    //    wgpu::BindGroupLayoutEntry e{};
    //    e.binding = binding;
    //    e.visibility = visibility;
    //    if ((m_usage & wgpu::BufferUsage::Uniform) == wgpu::BufferUsage::Uniform) {
    //        e.buffer.type = wgpu::BufferBindingType::Uniform;
    //    }
    //    else if ((m_usage & wgpu::BufferUsage::Storage) == wgpu::BufferUsage::Storage) {
    //        // Check if read-only or read-write at shader level is decided by ParameterSet entry
    //        e.buffer.type = wgpu::BufferBindingType::Storage;
    //    }
    //    else {
    //        // default to read-only storage if not uniform & not storage explicitly
    //        e.buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
    //    }
    //    e.buffer.hasDynamicOffset = false;
    //    e.buffer.minBindingSize = 0;
    //    return e;
    //}

    //wgpu::BindGroupEntry Buffer::bindGroupEntry(uint32_t binding) const {
    //    wgpu::BindGroupEntry ent{};
    //    ent.binding = binding;
    //    ent.buffer = m_buffer;
    //    ent.offset = 0;
    //    ent.size = static_cast<uint64_t>(m_size);
    //    return ent;
    //}

} // namespace krnl
