#pragma once
#include <webgpu/webgpu_cpp.h>
#include <cstddef>
#include <vector>
#include <functional>
#include <memory>
#include <string>

#include "core/device.hpp"
#include "core/future.hpp"

namespace krnl {

    enum class MapMode : uint64_t {
		None = wgpu::MapMode::None,
        Read = wgpu::MapMode::Read,
        Write = wgpu::MapMode::Write,
	};

    enum class BufferUsageType : uint64_t {
        Storage = wgpu::BufferUsage::Storage,
        Uniform = wgpu::BufferUsage::Uniform,
        CopySrc =wgpu::BufferUsage::CopySrc,
        CopyDst = wgpu::BufferUsage::CopyDst,
        MapRead = wgpu::BufferUsage::MapRead,
        MapWrite =wgpu::BufferUsage::MapWrite
    };

    inline BufferUsageType operator|(BufferUsageType a, BufferUsageType b) {
        return static_cast<BufferUsageType>(
            static_cast<uint64_t>(a) | static_cast<uint64_t>(b)
        );
    }

    inline BufferUsageType& operator|=(BufferUsageType& a, BufferUsageType b) {
        a = a | b;
        return a;
    }

    inline BufferUsageType operator&(BufferUsageType a, BufferUsageType b) {
        return static_cast<BufferUsageType>(
            static_cast<uint64_t>(a) & static_cast<uint64_t>(b)
        );
    }

    class Buffer {
    public:
        using ReadCallback = std::function<void(const void* data, size_t size)>;

        Buffer() = default;
		Buffer(const Device& device, size_t sizeBytes, BufferUsageType usage, std::string label , bool mappedAtCreation = false);

		Future MapAsync(MapMode mode, size_t offset, size_t size ,void* data);
		void WriteBuffer(const void* src, size_t bytes, size_t dstOffset = 0);

        const wgpu::Buffer GetNative() const { return m_Buffer; }
        size_t GetSize() const { return m_size; }

        //// High-performance write using mapped staging (create MapWrite staging, copy, submit)
        void WriteViaStaging(const void* src, size_t bytes);

        //// Async readback: copies into MapRead staging, maps it and calls cb with mapped data.
        void ReadAsync(ReadCallback cb);

        //// Utility
        uint64_t sizeAlignedToUniform() const {
            constexpr uint64_t ALIGN = 256u;
            uint64_t s = static_cast<uint64_t>(m_size);
            return ((s + ALIGN - 1) / ALIGN) * ALIGN;
        }

    private:
         wgpu::BufferDescriptor makeDesc(size_t size, wgpu::BufferUsage usage, const char* label, bool mappedAtCreation = false);

    private:
        BufferUsageType m_BufferUsageType;
		std::string m_Label;
        const Device& m_Device;
        size_t m_size = 0;
        wgpu::Buffer m_Buffer;
    };

} // namespace krnl
