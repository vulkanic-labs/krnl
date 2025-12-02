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


    Buffer::Buffer(const Device& device, size_t sizeBytes, BufferUsageType usage ,std::string label , bool mappedAtCreation)
        : m_Device(device), m_size(sizeBytes), m_BufferUsageType(usage), m_Label(label), m_Buffer(nullptr) {

        wgpu::BufferDescriptor desc = makeDesc(
            sizeBytes,
            static_cast<wgpu::BufferUsage>(usage),
            m_Label.c_str(),
            mappedAtCreation
        );

        m_Buffer = m_Device.GetNative().CreateBuffer(&desc);
    }

    Future Buffer::MapAsync(MapMode mode, size_t offset, size_t size ,void* data) {
        assert(m_Buffer);
		wgpu::MapMode wgpuMode = static_cast<wgpu::MapMode>(mode);
        return m_Buffer.MapAsync(wgpuMode, 0, size, wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::MapAsyncStatus status, wgpu::StringView message) {
                if (status == wgpu::MapAsyncStatus::Success) {
                    const void* mapped = m_Buffer.GetConstMappedRange(offset, size);
                    memcpy(data, mapped, size);
					KRNL_LOG("Buffer mapped successfully");
                }
                else {
					KRNL_ERROR("Buffer mapping failed: " << message);
                }
            });
	}

    void Buffer::WriteBuffer(const void* src, size_t bytes, size_t dstOffset) {
        assert(m_Buffer);
        if (bytes + dstOffset > m_size) {
            KRNL_ERROR("Buffer::WriteBuffer => out of range write (requested " << bytes << " bytes at offset " << dstOffset << ", buffer size " << m_size << ")");
            return;
        }
        m_Device.GetNative().GetQueue().WriteBuffer(m_Buffer, static_cast<uint64_t>(dstOffset), src, bytes);
	}

    ///* writeViaStaging */
    void Buffer::WriteViaStaging(const void* src, size_t bytes) {
        assert(m_Buffer);
        if (bytes > m_size) {
            KRNL_ERROR("Buffer::writeViaStaging => bytes  > buffer size "<< bytes <<m_size);
            return;
        }

        wgpu::BufferDescriptor desc = makeDesc(bytes, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, "krnl_staging_upload", true);
        wgpu::Buffer staging = m_Device.GetNative().CreateBuffer(&desc);

        void* mapped = staging.GetMappedRange();
        if (!mapped) {
            KRNL_ERROR("Buffer::writeViaStaging => staging.GetMappedRange() returned null");
            return;
        }
        std::memcpy(mapped, src, bytes);
        staging.Unmap();

        wgpu::CommandEncoderDescriptor encoderDesc{};
        wgpu::CommandEncoder encoder = m_Device.GetNative().CreateCommandEncoder(&encoderDesc);
        encoder.CopyBufferToBuffer(staging, 0, m_Buffer, 0, bytes);
        wgpu::CommandBuffer cmd = encoder.Finish();
        m_Device.getQueue().Submit(1, &cmd);
    }

    ///* readAsync */
    void Buffer::ReadAsync(ReadCallback cb) {
        assert(m_Buffer);

        // create staging read (MapRead | CopyDst)
        wgpu::BufferDescriptor desc = makeDesc(m_size, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, "krnl_staging_read", false);
        wgpu::Buffer staging = m_Device.GetNative().CreateBuffer(&desc);

        // encode copy
        wgpu::CommandEncoderDescriptor encoderDesc{};
        wgpu::CommandEncoder encoder = m_Device.GetNative().CreateCommandEncoder(&encoderDesc);
        encoder.CopyBufferToBuffer(m_Buffer, 0, staging, 0, m_size);
        wgpu::CommandBuffer cmd = encoder.Finish();
        m_Device.getQueue().Submit(1, &cmd);

        auto stagingPtr = std::make_shared<wgpu::Buffer>(staging);
        stagingPtr->MapAsync(
            wgpu::MapMode::Read,
            0,
            m_size,
            wgpu::CallbackMode::WaitAnyOnly,
            [stagingPtr, cb, this](wgpu::MapAsyncStatus status, wgpu::StringView message) {
                if (status != wgpu::MapAsyncStatus::Success) {
                    KRNL_ERROR("Buffer::readAsync -> MapAsync failed"<< status <<
                       message);
                    return;
                }
                const void* mapped = stagingPtr->GetConstMappedRange(0, this->m_size);
                if (!mapped) {
                    KRNL_ERROR("Buffer::readAsync -> GetConstMappedRange returned null");
                    stagingPtr->Unmap();
                    return;
                }
                cb(mapped, this->m_size);
                stagingPtr->Unmap();
            }
        );
    }


} // namespace krnl
