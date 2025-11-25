#include "core/buffer.hpp"
#include "core/log.h" // optional logging
#include <cstring>
#include <cassert>

namespace krnl {

	/* -----------------------
	   Helpers: usage mapping
	   ----------------------- */
	wgpu::BufferUsage Buffer::usageForType(Type t) {
		using BU = wgpu::BufferUsage;
		switch (t) {
		case Type::Storage:
			return BU::Storage | BU::CopyDst | BU::CopySrc;
		case Type::Uniform:
			return BU::Uniform | BU::CopyDst;
		case Type::MapRead:
			// This is intended as a GPU-side buffer that can be copied-to a mapread staging buffer.
			return BU::CopySrc;
		case Type::MapWrite:
			// MapWrite usually is used only for staging buffers that are MapWrite|CopySrc
			return BU::MapWrite | BU::CopySrc;
		case Type::Staging:
			// Generic staging is MapWrite|CopySrc (to upload) OR MapRead|CopyDst (for readback).
			// Caller should create the correct usage via CreateStaging(forWrite=true/false).
			return BU::MapWrite | BU::CopySrc;
		default:
			return BU::None;
		}
	}

	wgpu::BufferDescriptor Buffer::makeDesc(size_t size, wgpu::BufferUsage usage, const char* label, bool mappedAtCreation) {
		wgpu::BufferDescriptor desc{};
		desc.size = size;
		desc.usage = usage;
		desc.mappedAtCreation = mappedAtCreation;
		desc.label = label ? label : nullptr;
		return desc;
	}

	/* -----------------------
	   Constructors / factories
	   ----------------------- */

	Buffer::Buffer(wgpu::Device device, wgpu::Buffer buffer, size_t sizeBytes, Type type)
		: m_device(device), m_buffer(buffer), m_size(sizeBytes), m_type(type) {
	}

	Buffer Buffer::CreateStorage(wgpu::Device device, size_t sizeBytes, const char* label) {
		auto usage = usageForType(Type::Storage);
		auto desc = makeDesc(sizeBytes, usage, label, false);
		wgpu::Buffer b = device.CreateBuffer(&desc);
		Buffer out(device, b, sizeBytes, Type::Storage);
		return out;
	}

	Buffer Buffer::CreateUniform(wgpu::Device device, size_t sizeBytes, const char* label) {
		auto usage = usageForType(Type::Uniform);
		auto desc = makeDesc(sizeBytes, usage, label, false);
		wgpu::Buffer b = device.CreateBuffer(&desc);
		return Buffer(device, b, sizeBytes, Type::Uniform);
	}

	Buffer Buffer::CreateMapRead(wgpu::Device device, size_t sizeBytes, const char* label) {
		// MapRead buffer is meant to be a GPU-buffer that can be copied FROM into a MapRead staging buffer.
		// Here we create a normal GPU buffer (CopySrc) and callers will create staging buffers for readback.
		auto desc = makeDesc(sizeBytes, wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Storage, label, false);
		wgpu::Buffer b = device.CreateBuffer(&desc);
		return Buffer(device, b, sizeBytes, Type::MapRead);
	}

	Buffer Buffer::CreateMapWrite(wgpu::Device device, size_t sizeBytes, const char* label) {
		// A map-write buffer as a persistent GPU buffer is not portable; instead we create a staging buffer that is MapWrite|CopySrc.
		auto desc = makeDesc(sizeBytes, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, label, true);
		wgpu::Buffer b = device.CreateBuffer(&desc);
		return Buffer(device, b, sizeBytes, Type::MapWrite);
	}

	Buffer Buffer::CreateStaging(wgpu::Device device, size_t sizeBytes, bool forWrite, const char* label) {
		if (forWrite) {
			// MapWrite + CopySrc (staging for uploads)
			auto desc = makeDesc(sizeBytes, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, label, true);
			wgpu::Buffer b = device.CreateBuffer(&desc);
			return Buffer(device, b, sizeBytes, Type::Staging);
		}
		else {
			// MapRead + CopyDst (staging for readback)
			auto desc = makeDesc(sizeBytes, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, label, false);
			wgpu::Buffer b = device.CreateBuffer(&desc);
			return Buffer(device, b, sizeBytes, Type::Staging);
		}
	}

	/* -----------------------
	   Set label
	   ----------------------- */
	void Buffer::setLabel(const char* label) {
		if (m_buffer) {
			m_buffer.SetLabel(label ? wgpu::StringView(label) : wgpu::StringView(nullptr));
		}
	}

	/* -----------------------
	   write (fast): queue.WriteBuffer
	   ----------------------- */
	void Buffer::write(const void* src, size_t bytes, size_t dstOffset) {
		assert(m_buffer);
		if (bytes + dstOffset > m_size) {
			KRNL_ERROR("Buffer::write => out of range write (requested %zu bytes at offset %zu, buffer size )" << bytes << " " << dstOffset << " " << m_size);
			return;
		}
		// Use queue WriteBuffer via device queue
		// NOTE: WriteBuffer is a convenience; for large transfers use writeViaStaging.
		m_device.GetQueue().WriteBuffer(m_buffer, static_cast<uint64_t>(dstOffset), src, bytes);
	}

	/* -----------------------
	   writeViaStaging (fast large uploads)
	   - Creates a staging MapWrite|CopySrc buffer with mappedAtCreation = true
	   - Copies into staging's mapped range, unmaps, issues CopyBufferToBuffer to the destination, submits.
	   ----------------------- */
	void Buffer::writeViaStaging(const void* src, size_t bytes, wgpu::Queue queue) {
		assert(m_buffer);
		if (bytes > m_size) {
			KRNL_ERROR("Buffer::writeViaStaging => bytes (%zu) > buffer size "<< bytes << " " << m_size);
			return;
		}

		// Create staging buffer mappedAtCreation
		wgpu::BufferDescriptor desc = makeDesc(bytes, wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, "krnl_staging_upload", true);
		wgpu::Buffer staging = m_device.CreateBuffer(&desc);

		// Copy into mapped range
		void* mapped = staging.GetMappedRange();
		if (!mapped) {
			KRNL_ERROR("Buffer::writeViaStaging => staging.GetMappedRange() returned null");
			return;
		}
		std::memcpy(mapped, src, bytes);
		staging.Unmap();

		// Encode copy
		wgpu::CommandEncoderDescriptor encoderDesc{};
		wgpu::CommandEncoder encoder = m_device.CreateCommandEncoder(&encoderDesc);
		encoder.CopyBufferToBuffer(staging, 0, m_buffer, 0, bytes);
		wgpu::CommandBuffer cmd = encoder.Finish();
		queue.Submit(1, &cmd);

		// Keep staging alive until the GPU consumes it; capture in lambda if you plan to wait on fences.
		// Here we assume queue submission will keep backing resources valid until GPU done (Dawn/WT handles lifetime).
		// If you want to guarantee device.WaitIdle-like semantics, wait on a future/ fence.
	}

	/* -----------------------
	   readAsync
	   - Creates a MapRead staging buffer (MapRead|CopyDst)
	   - Copies from GPU buffer to staging buffer, submits, then MapAsync on staging
	   - Calls callback with mapped data pointer
	   ----------------------- */
	void Buffer::readAsync(wgpu::Device device, wgpu::Queue queue, ReadCallback cb) {
		assert(m_buffer);
		// Create staging read buffer (MapRead | CopyDst)
		wgpu::BufferDescriptor desc = makeDesc(m_size, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, "krnl_staging_read", false);
		wgpu::Buffer staging = device.CreateBuffer(&desc);

		// Encode copy: this -> staging
		wgpu::CommandEncoderDescriptor encoderDesc{};
		wgpu::CommandEncoder encoder = device.CreateCommandEncoder(&encoderDesc);
		encoder.CopyBufferToBuffer(m_buffer, 0, staging, 0, m_size);
		wgpu::CommandBuffer cmd = encoder.Finish();
		queue.Submit(1, &cmd);

		// To ensure staging object stays alive until callback runs, create a shared_ptr and capture it.
		auto stagingPtr = std::make_shared<wgpu::Buffer>(staging);

		// MapAsync with callback
		// Note: callback signature: void(WGPUMapAsyncStatus status, char const* message)
		// Using wgpu::CallbackMode::WaitAnyOnly is fine; adapt if you need different behavior.
		stagingPtr->MapAsync(
			wgpu::MapMode::Read,
			0,
			m_size,
			wgpu::CallbackMode::WaitAnyOnly,
			// capture stagingPtr and cb by value to keep both alive
			[stagingPtr, cb, this](wgpu::MapAsyncStatus status, wgpu::StringView message) {
				if (status != wgpu::MapAsyncStatus::Success) {
					KRNL_ERROR("Buffer::readAsync -> MapAsync failed (status %d). msg: %.*s" << status << " " << message);
					return;
				}
				const void* mapped = stagingPtr->GetConstMappedRange(0, this->m_size);
				if (!mapped) {
					KRNL_ERROR("Buffer::readAsync -> GetConstMappedRange returned null");
					stagingPtr->Unmap();
					return;
				}
				// Call user callback with the mapped pointer and size
				cb(mapped, this->m_size);
				stagingPtr->Unmap();
				// stagingPtr will go out of scope and free when lambda ends
			}
		);
	}

	/* -----------------------
	   Bind group helpers
	   ----------------------- */
	wgpu::BindGroupLayoutEntry Buffer::bindGroupLayoutEntry(uint32_t binding, wgpu::ShaderStage visibility) const {
		wgpu::BindGroupLayoutEntry e{};
		e.binding = binding;
		e.visibility = visibility;

		// If buffer type is Uniform -> Uniform, else Storage
		if (m_type == Type::Uniform) {
			e.buffer.type = wgpu::BufferBindingType::Uniform;
		}
		else {
			// default: Storage
			e.buffer.type = wgpu::BufferBindingType::Storage;
		}

		// offset / size are unused in BGL entries
		e.buffer.hasDynamicOffset = false;
		e.buffer.minBindingSize = 0;
		return e;
	}

	wgpu::BindGroupEntry Buffer::bindGroupEntry(uint32_t binding) const {
		wgpu::BindGroupEntry ent{};
		ent.binding = binding;
		ent.buffer = m_buffer;
		ent.offset = 0;
		ent.size = static_cast<uint64_t>(m_size);
		return ent;
	}

} // namespace krnl
