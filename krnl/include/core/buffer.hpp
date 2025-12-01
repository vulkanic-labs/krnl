#pragma once
#include <webgpu/webgpu_cpp.h>
#include "core/device.hpp"
#include <cstddef>
#include <vector>
#include <functional>
#include <memory>
#include <string>

namespace krnl {

    enum class BufferUsageType : uint64_t {
        Storage = static_cast<uint64_t>(wgpu::BufferUsage::Storage),
        Uniform = static_cast<uint64_t>(wgpu::BufferUsage::Uniform),
        CopySrc = static_cast<uint64_t>(wgpu::BufferUsage::CopySrc),
        CopyDst = static_cast<uint64_t>(wgpu::BufferUsage::CopyDst),
        MapRead = static_cast<uint64_t>(wgpu::BufferUsage::MapRead),
        MapWrite = static_cast<uint64_t>(wgpu::BufferUsage::MapWrite)
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
        //enum class Type {
        //    Storage,
        //    Uniform,
        //    MapRead,   // GPU buffer intended to be copied from (CopySrc)
        //    MapWrite,  // staging buffer mapped at creation for uploads (MapWrite|CopySrc)
        //    Staging     // generic staging
        //};

        //using ReadCallback = std::function<void(const void* data, size_t size)>;

        // Factories
        //static Buffer CreateStorage(wgpu::Device device, size_t sizeBytes, const char* label = nullptr , wgpu::BufferUsage usageflag = wgpu::BufferUsage::None);
        /*static Buffer CreateUniform(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
        static Buffer CreateMapRead(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
        static Buffer CreateMapWrite(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
        static Buffer CreateStaging(wgpu::Device device, size_t sizeBytes, bool forWrite, const char* label = nullptr);*/

        Buffer() = default;
        //Buffer(wgpu::Device device, wgpu::Buffer buffer, size_t sizeBytes, Type type, wgpu::BufferUsage usage);
		Buffer(Device& device, size_t sizeBytes, BufferUsageType usage, std::string label , bool mappedAtCreation = false);

        wgpu::Buffer GetNative() const { return m_Buffer; }
        size_t GetSize() const { return m_size; }

        //// Fast write (queue.WriteBuffer). Good for small updates.
        //void write(const void* src, size_t bytes, size_t dstOffset = 0);

        //// High-performance write using mapped staging (create MapWrite staging, copy, submit)
        //void writeViaStaging(const void* src, size_t bytes, wgpu::Queue queue);

        //// Async readback: copies into MapRead staging, maps it and calls cb with mapped data.
        //void readAsync(wgpu::Device device, wgpu::Queue queue, ReadCallback cb);


        //// Helpers for bind creation
        //wgpu::BindGroupLayoutEntry bindGroupLayoutEntry(uint32_t binding, wgpu::ShaderStage visibility) const;
        //wgpu::BindGroupEntry bindGroupEntry(uint32_t binding) const;

        //// Utility
        //uint64_t sizeAlignedToUniform() const {
        //    constexpr uint64_t ALIGN = 256u;
        //    uint64_t s = static_cast<uint64_t>(m_size);
        //    return ((s + ALIGN - 1) / ALIGN) * ALIGN;
        //}

        //// Label
        //void setLabel(const char* label);

    private:
        //static wgpu::BufferUsage usageForType(Type t, bool forWrite = true);
         wgpu::BufferDescriptor makeDesc(size_t size, wgpu::BufferUsage usage, const char* label, bool mappedAtCreation = false);

    private:
        BufferUsageType m_BufferUsageType;
		std::string m_Label;
        Device& m_Device;
        size_t m_size = 0;
        wgpu::Buffer m_Buffer;
    };

} // namespace krnl
