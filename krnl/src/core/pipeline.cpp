#include "core/pipeline.hpp"
#include "core/shader.hpp" // loadWGSL helper
#include "core/log.h"
#include <cassert>

namespace krnl {

	/* static factory: from shader module */
	Pipeline Pipeline::CreateCompute(
		const Device& device,
		const Shader& module,
		const ParameterSet& params,
		const std::string& entryPoint,
		const char* label)
	{
		Pipeline p(device , params); // Use new constructor to initialize m_Device
		p.m_ShaderModule = module.GetNative();
		p.buildPipeline(p.m_ShaderModule, entryPoint, label);
		return p;
	}

	/* internal helper to build pipeline layout and pipeline */
	void Pipeline::buildPipeline(const wgpu::ShaderModule& module, const std::string& entryPoint, const char* label) {
		assert(&m_Params != nullptr && "ParameterSet must be provided");

		wgpu::PipelineLayoutDescriptor pipelineLayoutDesc{};
		pipelineLayoutDesc.bindGroupLayoutCount = 1;
		pipelineLayoutDesc.bindGroupLayouts = &m_Params.layout();
		m_PipelineLayout = m_Device.GetNative().CreatePipelineLayout(&pipelineLayoutDesc);

		// Create compute pipeline
		wgpu::ComputePipelineDescriptor pipelineDesc{};
		pipelineDesc.layout = m_PipelineLayout;
		pipelineDesc.compute.module = module;
		pipelineDesc.compute.entryPoint = entryPoint.c_str();
		if (label) {
			pipelineDesc.label = label;
		}

		if (!m_Device.IsValid()) {
			KRNL_ERROR("Cannot build pipeline: invalid device");
			std::exit(EXIT_FAILURE);
		}
		m_Pipeline = m_Device.GetNative().CreateComputePipeline(&pipelineDesc);
	}

	void Pipeline::encodeDispatch(const CommandList& cmd, uint32_t x, uint32_t y, uint32_t z) {
		cmd.GetComputePass().SetPipeline(m_Pipeline);
		cmd.GetComputePass().SetBindGroup(0, m_Params.bindGroup(), 0, nullptr);
		cmd.GetComputePass().DispatchWorkgroups(x, y, z);
	}
} // namespace krnl
