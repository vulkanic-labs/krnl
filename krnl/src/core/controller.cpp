#include "core/controller.hpp"

#include "core/context.hpp"
#include "core/device.hpp"
#include "core/shader.hpp"
#include "core/log.h"

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include <iostream>

namespace krnl
{
	Controller::Controller()
	{
		wgpu::Instance instance = m_context.getInstance();
		m_device = std::make_unique<Device>(instance);
		wgpu::Queue queue = m_device->getQueue();

		wgpu::ShaderModule computeShadeModule = loadWGSL(m_device->get(), R"(
			@group(0) @binding(0) var<storage,read> inputBuffer: array<f32,64>;
			@group(0) @binding(1) var<storage,read_write> outputBuffer: array<f32,64>;
			@compute @workgroup_size(32)
			fn main(@builtin(global_invocation_id) id: vec3<u32>) {
				// Apply the function f to the buffer element at index id.x:
				outputBuffer[id.x] = inputBuffer[id.x] + 1;
			}
		)");

		//TEST CODES
		// initBindGroupLayout
		std::vector<wgpu::BindGroupLayoutEntry> bindings(2);
		// Input buffer
		bindings[0].binding = 0;
		bindings[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
		bindings[0].visibility = wgpu::ShaderStage::Compute;

		// Output buffer
		bindings[1].binding = 1;
		bindings[1].buffer.type = wgpu::BufferBindingType::Storage;
		bindings[1].visibility = wgpu::ShaderStage::Compute;

		wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
		bindGroupLayoutDesc.entryCount = (uint32_t)bindings.size();
		bindGroupLayoutDesc.entries = bindings.data();
		wgpu::BindGroupLayout bindGroupLayout = m_device->get().CreateBindGroupLayout(&bindGroupLayoutDesc); // Promote to privte

		wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
		pipelineLayoutDesc.bindGroupLayoutCount = 1;
		pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
		wgpu::PipelineLayout m_pipelineLayout = m_device->get().CreatePipelineLayout(&pipelineLayoutDesc);


		float m_bufferSize = 64 * sizeof(float);

		wgpu::BufferDescriptor bufferDesc;
		bufferDesc.mappedAtCreation = false;
		bufferDesc.size = m_bufferSize;
		bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
		wgpu::Buffer inputBuffer = m_device->get().CreateBuffer(&bufferDesc);


		// Create output buffer: the only difference is the usage
		bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
		wgpu::Buffer outputBuffer = m_device->get().CreateBuffer(&bufferDesc);

		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
		wgpu::Buffer m_mapBuffer = m_device->get().CreateBuffer(&bufferDesc);

		std::vector<float> input(m_bufferSize / sizeof(float));
		for (int i = 0; i < input.size(); ++i) {
			input[i] = 0.1f * i;
		}
		queue.WriteBuffer(inputBuffer, 0, input.data(), m_bufferSize);

		// Create bind group
		// Create compute bind group
		std::vector<wgpu::BindGroupEntry> entries(2);

		// Input buffer
		entries[0].binding = 0;
		entries[0].buffer = inputBuffer;
		entries[0].offset = 0;
		entries[0].size = m_bufferSize;

		// Output buffer
		entries[1].binding = 1;
		entries[1].buffer = outputBuffer;
		entries[1].offset = 0;
		entries[1].size = m_bufferSize;

		wgpu::BindGroupDescriptor bindGroupDesc;
		bindGroupDesc.layout = bindGroupLayout;
		bindGroupDesc.entryCount = (uint32_t)entries.size();
		bindGroupDesc.entries = entries.data();
		wgpu::BindGroup m_bindGroup = m_device->get().CreateBindGroup(&bindGroupDesc);



		wgpu::CommandEncoderDescriptor encoderDesc{};
		wgpu::CommandEncoder encoder =
			m_device->get().CreateCommandEncoder(&encoderDesc);

		wgpu::ComputePassDescriptor computePassDesc{};
		computePassDesc.timestampWrites = nullptr;
		wgpu::ComputePassEncoder computePass = encoder.BeginComputePass(&computePassDesc);

		wgpu::ComputePipelineDescriptor computePipelineDesc{};
		computePipelineDesc.compute.entryPoint = "main";
		computePipelineDesc.compute.module = computeShadeModule;
		computePipelineDesc.layout = m_pipelineLayout;

		wgpu::ComputePipeline computePipeline =
			m_device->get().CreateComputePipeline(&computePipelineDesc);

		computePass.SetPipeline(computePipeline);
		computePass.SetBindGroup(0, m_bindGroup, 0, nullptr);

		uint32_t invocationCount = m_bufferSize / sizeof(float);
		uint32_t workgroupSize = 32;
		uint32_t workgroupCount = (invocationCount + workgroupSize - 1) / workgroupSize;


		computePass.DispatchWorkgroups(workgroupCount, 1, 1);
		computePass.End();

		encoder.CopyBufferToBuffer(outputBuffer, 0, m_mapBuffer, 0, m_bufferSize);

		wgpu::CommandBufferDescriptor cmdBufDesc{};
		cmdBufDesc.label = "Compute Command Buffer";
		wgpu::CommandBuffer commands = encoder.Finish(&cmdBufDesc);
		queue.Submit(1, &commands);

		wgpu::Future f = m_mapBuffer.MapAsync(wgpu::MapMode::Read, 0, m_bufferSize, wgpu::CallbackMode::WaitAnyOnly, [&](wgpu::MapAsyncStatus status, wgpu::StringView message) {
			if (status == wgpu::MapAsyncStatus::Success) {
				const float* output = (const float*)m_mapBuffer.GetConstMappedRange(0, m_bufferSize);
				for (int i = 0; i < input.size(); ++i) {
					std::cout << i + 1 << " : input " << input[i] << " became " << output[i] << std::endl;
					std::cout << ((input[i] + 1 == output[i]) ? "PASS" : "FAIL") << std::endl;
				}
				m_mapBuffer.Unmap();
			}
		
			});

		/*while (!done)
		{
			instance.ProcessEvents();
		}*/

		instance.WaitAny(f, UINT64_MAX);

		while (true) {
			instance.ProcessEvents();
		}

		//release resources
		inputBuffer.Destroy();
		outputBuffer.Destroy();
		m_mapBuffer.Destroy();
		m_device->get().Destroy();
	}

	Controller::~Controller() = default;
}