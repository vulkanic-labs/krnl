#pragma once
#include <webgpu/webgpu_cpp.h>
#include <cstddef>
#include <vector>
#include <functional>
#include <memory>
#include <type_traits>

namespace krnl {

	class Buffer {
	public:
		enum class Type {
			Storage,
			Uniform,
			MapRead,   // mappable for reading back to CPU
			MapWrite,  // mappable for CPU writes (staging)
			Staging     // generic staging buffer (MapWrite | CopySrc or MapRead | CopyDst)
		};

		using ReadCallback = std::function<void(const void* data, size_t size)>;

		// Factories
		static Buffer CreateStorage(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
		static Buffer CreateUniform(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
		static Buffer CreateMapRead(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
		static Buffer CreateMapWrite(wgpu::Device device, size_t sizeBytes, const char* label = nullptr);
		static Buffer CreateStaging(wgpu::Device device, size_t sizeBytes, bool forWrite, const char* label = nullptr);

		Buffer() = default;
		// Construct from an already-created wgpu::Buffer (rare)
		Buffer(wgpu::Device device, wgpu::Buffer buffer, size_t sizeBytes, Type type);

		// Fast write: uses Queue.WriteBuffer (synchronous from API perspective)
		// Good for small updates
		void write(const void* src, size_t bytes, size_t dstOffset = 0);

		// High-performance write: create a MapWrite staging buffer, copy, submit.
		// Good for large uploads (uses mappedAtCreation)
		// This call blocks only until staging Unmap; submission is asynchronous.
		void writeViaStaging(const void* src, size_t bytes, wgpu::Queue queue);

		// Async readback: copies this buffer into a MapRead staging buffer and maps it,
		// calling cb when data is available. cb will be invoked on the thread executing WebGPU callbacks.
		// Important: callback must not destroy the device during execution.
		void readAsync(wgpu::Device device, wgpu::Queue queue, ReadCallback cb);

		// Access underlying buffer
		wgpu::Buffer get() const { return m_buffer; }
		size_t size() const { return m_size; }
		Type type() const { return m_type; }

		// Bind helpers so callers can build bind group layouts / bind groups easily
		wgpu::BindGroupLayoutEntry bindGroupLayoutEntry(uint32_t binding, wgpu::ShaderStage visibility) const;
		wgpu::BindGroupEntry bindGroupEntry(uint32_t binding) const;

		// Optional label for debugging
		void setLabel(const char* label);

		// Fix: Use underlying usage flags for bitwise operations, not enum class Type
		bool isUniform() const { return (m_usage & wgpu::BufferUsage::Uniform) != 0; }
		bool isStorageRW() const {
			return (m_usage & wgpu::BufferUsage::Storage) != 0 &&
				(m_usage & wgpu::BufferUsage::CopyDst) != 0;
		}

		uint64_t size() const { return m_size; }

		// Uniform buffers must be aligned to 256 bytes
		uint64_t sizeAlignedToUniform() const {
			uint64_t align = 256;
			return ((m_size + align - 1) / align) * align;
		}

		const wgpu::Buffer& wgpuBuffer() const { return m_buffer; }


	private:
		// internal helpers
		static wgpu::BufferUsage usageForType(Type t);
		static wgpu::BufferDescriptor makeDesc(size_t size, wgpu::BufferUsage usage, const char* label, bool mappedAtCreation = false);

	private:
		wgpu::Buffer m_buffer;
		wgpu::Device m_device;
		size_t m_size = 0;
		Type m_type = Type::Storage;
		wgpu::BufferUsage m_usage = wgpu::BufferUsage::None; // Add this field to store usage flags
	};
} // namespace krnl
