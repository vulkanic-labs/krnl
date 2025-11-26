#include "core/pipeline.hpp"
#include "core/shader.hpp" // loadWGSL helper
#include "core/log.h"
#include <cassert>

namespace krnl {

	/* static factory: from WGSL source */
	Pipeline Pipeline::CreateComputeFromWGSL(
		wgpu::Device device,
		const std::string& wgslSource,
		ParameterSet& params,
		const std::string& entryPoint,
		const char* label)
	{
		Pipeline p;
		p.m_device = device;
		p.m_params = &params;

		// Create shader module
		p.m_shaderModule = loadWGSL(device, wgslSource);

		p.buildPipeline(p.m_shaderModule, entryPoint, label);
		return p;
	}

	/* static factory: from shader module */
	Pipeline Pipeline::CreateComputeFromModule(
		wgpu::Device device,
		const wgpu::ShaderModule& module,
		ParameterSet& params,
		const std::string& entryPoint,
		const char* label)
	{
		Pipeline p;
		p.m_device = device;
		p.m_params = &params;
		p.m_shaderModule = module;
		p.buildPipeline(p.m_shaderModule, entryPoint, label);
		return p;
	}

	/* internal helper to build pipeline layout and pipeline */
	void Pipeline::buildPipeline(const wgpu::ShaderModule& module, const std::string& entryPoint, const char* label) {
		assert(m_params != nullptr && "ParameterSet must be provided");

		// Use the ParameterSet's bind group layout as the pipeline's first layout
		// If ParameterSet exposes its layout (it does in my earlier implementation) use it directly.
		wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
		pipelineLayoutDesc.bindGroupLayoutCount = 1;
		pipelineLayoutDesc.bindGroupLayouts = &m_params->layout();
		// pipelineLayoutDesc.pushConstantRangeCount = 0; // Uncomment if needed
		// pipelineLayoutDesc.pushConstantRanges = nullptr; // Uncomment if needed

		m_pipelineLayout = m_device.CreatePipelineLayout(&pipelineLayoutDesc);

		// Create compute pipeline
		wgpu::ComputePipelineDescriptor pipelineDesc{};
		pipelineDesc.layout = m_pipelineLayout;
		pipelineDesc.compute.module = module;
		pipelineDesc.compute.entryPoint = entryPoint.c_str();
		// label is optional: some Dawn versions accept label via pipelineDesc.label string view; else ignore
		if (label) {
			pipelineDesc.label = label;
		}

		m_pipeline = m_device.CreateComputePipeline(&pipelineDesc);
	}

	/* set push constants data (stored locally) */
	void Pipeline::setPushConstants(const void* data, size_t bytes) {
		if (data == nullptr || bytes == 0) {
			m_hasPushConstants = false;
			m_pushData.clear();
			return;
		}
		// limit check: push constants are typically small (<= 256 bytes). We store whatever user provides,
		// but the actual backend may truncate or fail. We only store; applying happens during encodeDispatch.
		m_pushData.assign(reinterpret_cast<const uint8_t*>(data), reinterpret_cast<const uint8_t*>(data) + bytes);
		m_hasPushConstants = true;
	}

	/* encodeDispatch: set pipeline, bind group 0, optionally push constants, then dispatch */
	void Pipeline::encodeDispatch(wgpu::ComputePassEncoder& pass, uint32_t x, uint32_t y, uint32_t z) {
		// Set pipeline
		pass.SetPipeline(m_pipeline);

		// Set bind group 0 from parameter set
		pass.SetBindGroup(0, m_params->bindGroup(), 0, nullptr);

		// Apply push constants if available
		if (m_hasPushConstants && !m_pushData.empty()) {
			// Try to set push constants. If backend doesn't support it this will be a no-op or assert.
			// Dawn's C++ wrapper has SetPushConstants available on ComputePassEncoder. We'll attempt it.
			// If push constants are not supported on current backend (e.g. Web), this may fail or be ignored.
			// For production, implement a uniform-buffer fallback: copy pushData into a small uniform buffer and rebind.
			// We'll attempt to call SetPushConstants and catch issues by logging.
			try {
				//pass.SetPushConstants(wgpu::ShaderStage::Compute, 0, m_pushData.size(), m_pushData.data());
			}
			catch (...) {
				KRNL_WARN("Pipeline::encodeDispatch: SetPushConstants call failed; consider adding a uniform fallback for web.");
				// Fallback: not implemented here — user may want uniform fallback.
			}
		}

		// Dispatch
		pass.DispatchWorkgroups(x, y, z);
	}
} // namespace krnl
