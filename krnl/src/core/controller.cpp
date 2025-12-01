#include "core/controller.hpp"

#include "core/device.hpp"
#include "core/shader.hpp"
#include "core/log.h"
#include "core/buffer.hpp"
#include "core/parameterset.hpp"
#include "core/pipeline.hpp"
#include "core/stagingpool.hpp"
#include "core/future.hpp"

#include <memory>
#include <webgpu/webgpu_cpp.h>
#include <iostream>

namespace krnl
{
	Controller::Controller()
	{
		m_device = Device(m_instance);
		wgpu::Queue queue = m_device.getQueue();

		Shader computeShader = Shader::loadWGSL(m_device, R"(
			@group(0) @binding(0) var<storage,read> inputBuffer: array<f32,64>;
			@group(0) @binding(1) var<storage,read_write> outputBuffer: array<f32,64>;
			@compute @workgroup_size(32)
			fn main(@builtin(global_invocation_id) id: vec3<u32>) {
				// Apply the function f to the buffer element at index id.x:
				outputBuffer[id.x] = inputBuffer[id.x] + 1;
			}
		)");

		size_t bufferSize = 64 * sizeof(float);
		Buffer input(m_device, bufferSize, BufferUsageType::Storage | BufferUsageType::CopyDst, "input");
		Buffer output(m_device, bufferSize, BufferUsageType::Storage | BufferUsageType::CopySrc, "output");
		Buffer map(m_device, bufferSize, BufferUsageType::CopyDst | BufferUsageType::MapRead, "map");

		std::vector<krnl::ParameterSet::Entry> paramEntries;
		paramEntries.push_back({ input,  krnl::BufferBindingType::ReadOnlyStorage });
		paramEntries.push_back({ output, krnl::BufferBindingType::Storage });
		ParameterSet params(m_device, paramEntries);

		auto pipeline = Pipeline::CreateCompute(
			m_device,
			computeShader,
			params,
			"main",
			"ComputeAddOne"
		);

		// Prepare input data
		std::vector<float> inputData(bufferSize / sizeof(float));
		for (size_t i = 0; i < inputData.size(); i++) {
			inputData[i] = static_cast<float>(i);
		}

		input.WriteBuffer(inputData.data(), bufferSize, 0);

		// Command encoder & compute pass
		wgpu::CommandEncoder encoder = m_device.GetNative().CreateCommandEncoder();
		wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
		uint32_t workgroups = (64 + 31) / 32;
		pipeline.encodeDispatch(pass, workgroups);
		pass.End();
		// Copy output to map buffer
		encoder.CopyBufferToBuffer(
			output.GetNative(), 0,
			map.GetNative(), 0,
			bufferSize
		);

		wgpu::CommandBuffer cmd = encoder.Finish();
		queue.Submit(1, &cmd);

		std::vector<float> outputData(bufferSize / sizeof(float), 0.0f);
		Future f = map.MapAsync(krnl::MapMode::Read, 0, bufferSize, outputData.data());
		m_instance.WaitAny(f, UINT64_MAX);

		for (size_t i = 0; i < inputData.size(); ++i)
			std::cout << i + 1 << " : input " << inputData[i] << " became " << outputData[i] << std::endl;

		m_instance.ProcessEvents();


		//// Readback
		//wgpu::Future f = map.GetNative().MapAsync(wgpu::MapMode::Read, 0, bufferSize, wgpu::CallbackMode::WaitAnyOnly,
		//	[&](wgpu::MapAsyncStatus status, wgpu::StringView message) {
		//		if (status == wgpu::MapAsyncStatus::Success) {
		//			const float* output = (const float*)map.GetNative().GetConstMappedRange(0, bufferSize);
		//			if (output) {
		//				for (size_t i = 0; i < inputData.size(); ++i)
		//					std::cout << i + 1 << " : input " << inputData[i] << " became " << output[i] << std::endl;
		//				map.GetNative().Unmap();
		//			}
		//			else {
		//				std::cerr << "GetConstMappedRange returned null\n";
		//			}
		//		}
		//		else {
		//			std::cerr << "MapAsync failed: " << /*message*/ "(empty)" << "\n";
		//		}
		//	});

		//m_instance.WaitAny(f, UINT64_MAX);

		// =====================================================================================
		// WGSL SHADER
		// =====================================================================================

	/*	static const char* WGSL_SRC = R"(
		struct Params {
			scale : f32,
			bias  : f32,
		};
		@group(0) @binding(0) var<storage, read>        inA     : array<f32>;
		@group(0) @binding(1) var<storage, read>        inB     : array<f32>;
		@group(0) @binding(2) var<storage, read_write>  outBuf  : array<f32>;
		@group(0) @binding(3) var<uniform>              params  : Params;

		@compute @workgroup_size(32)
		fn main(@builtin(global_invocation_id) gid : vec3<u32>) {
			let i = gid.x;
			outBuf[i] = (inA[i] + inB[i]) * params.scale + params.bias;
		}
	)";*/

	/*	struct ParamsCPU { float scale; float bias; };

		const uint32_t N = 64;
		std::vector<float> A(N), B(N);
		for (uint32_t i = 0; i < N; i++) {
			A[i] = float(i);
			B[i] = float(i * 2);
		}

		ParamsCPU paramsCPU{ 0.5f, 1.0f };*/

		// =====================================================================================
		// CREATE BUFFERS
		// =====================================================================================

	/*	Buffer bufA = Buffer::CreateStorage(m_device->get(), N * sizeof(float), "bufA");
		Buffer bufB = Buffer::CreateStorage(m_device->get(), N * sizeof(float), "bufB");
		Buffer bufOut = Buffer::CreateStorage(m_device->get(), N * sizeof(float), "bufOut");
		Buffer bufUniform = Buffer::CreateUniform(m_device->get(), sizeof(ParamsCPU), "params");

		queue.WriteBuffer(bufA.native(), 0, A.data(), A.size() * sizeof(float));
		queue.WriteBuffer(bufB.native(), 0, B.data(), B.size() * sizeof(float));
		queue.WriteBuffer(bufUniform.native(), 0, &paramsCPU, sizeof(ParamsCPU));*/

		// =====================================================================================
		// PARAMETER SET
		// =====================================================================================

		/*std::vector<krnl::ParameterSet::Entry> paramEntries;
		paramEntries.push_back({ &bufA,       wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage });
		paramEntries.push_back({ &bufB,       wgpu::ShaderStage::Compute, wgpu::BufferBindingType::ReadOnlyStorage });
		paramEntries.push_back({ &bufOut,     wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage });
		paramEntries.push_back({ &bufUniform, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Uniform });

		ParameterSet params(m_device->get(), paramEntries);*/

		// =====================================================================================
		// PIPELINE
		// =====================================================================================

	/*	auto pipeline = Pipeline::CreateComputeFromWGSL(
			m_device->get(), WGSL_SRC, params, "main", "ComputeAddMulBias"
		);*/

		// =====================================================================================
		// COMMAND ENCODER & COMPUTE PASS
		// =====================================================================================

		/*wgpu::CommandEncoder encoder = m_device->get().CreateCommandEncoder();
		wgpu::ComputePassEncoder pass = encoder.BeginComputePass();

		uint32_t workgroups = (N + 31) / 32;
		pipeline.encodeDispatch(pass, workgroups);

		pass.End();*/

		// =====================================================================================
		// PERSISTENT STAGING POOL
		// =====================================================================================

		//krnl::PersistentStagingPool stagingPool(m_device->get());

		// Allocate a staging buffer for readback
		/*auto stagingHandle = stagingPool.allocateForReadback(N * sizeof(float));

		encoder.CopyBufferToBuffer(
			bufOut.native(), 0,
			stagingHandle->buffer, 0,
			N * sizeof(float)
		);

		wgpu::CommandBuffer cmd = encoder.Finish();
		queue.Submit(1, &cmd);*/

		// =====================================================================================
		// ASYNC READBACK
		// =====================================================================================

	/*	bool done = false;
		std::vector<float> OUT1(N);

		stagingPool.readbackInto(
			bufOut.native(),
			N * sizeof(float),
			0,
			queue,
			[&](const void* data, size_t size) {
				std::memcpy(OUT1.data(), data, std::min(size, OUT1.size() * sizeof(float)));
				done = true;
			}
		);

		while (!done)
			instance.ProcessEvents();*/

			// =====================================================================================
			// CPU vs GPU validation
			// =====================================================================================

		/*	for (uint32_t i = 0; i < N; i++) {
				float cpu = (A[i] + B[i]) * paramsCPU.scale + paramsCPU.bias;
				std::cout << "i=" << i
					<< " GPU=" << OUT1[i]
					<< " CPU=" << cpu
					<< std::endl;
			}*/

			// =====================================================================================
			// CLEANUP
			// =====================================================================================

			/*bufA.native().Destroy();
			bufB.native().Destroy();
			bufOut.native().Destroy();
			bufUniform.native().Destroy();*/
			// staging buffer is managed by PersistentStagingPool
			//m_device->get().Destroy();

			//wgpu::ShaderModule computeShadeModule = loadWGSL(m_device->get(), R"(
			//	@group(0) @binding(0) var<storage,read> inputBuffer: array<f32,64>;
			//	@group(0) @binding(1) var<storage,read_write> outputBuffer: array<f32,64>;
			//	@compute @workgroup_size(32)
			//	fn main(@builtin(global_invocation_id) id: vec3<u32>) {
			//		// Apply the function f to the buffer element at index id.x:
			//		outputBuffer[id.x] = inputBuffer[id.x] + 1;
			//	}
			//)");

			////TEST CODES
			//// initBindGroupLayout
			//std::vector<wgpu::BindGroupLayoutEntry> bindings(2);
			//// Input buffer
			//bindings[0].binding = 0;
			//bindings[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
			//bindings[0].visibility = wgpu::ShaderStage::Compute;

			//// Output buffer
			//bindings[1].binding = 1;
			//bindings[1].buffer.type = wgpu::BufferBindingType::Storage;
			//bindings[1].visibility = wgpu::ShaderStage::Compute;

			//wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
			//bindGroupLayoutDesc.entryCount = (uint32_t)bindings.size();
			//bindGroupLayoutDesc.entries = bindings.data();
			//wgpu::BindGroupLayout bindGroupLayout = m_device->get().CreateBindGroupLayout(&bindGroupLayoutDesc); // Promote to privte

			//wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
			//pipelineLayoutDesc.bindGroupLayoutCount = 1;
			//pipelineLayoutDesc.bindGroupLayouts = &bindGroupLayout;
			//wgpu::PipelineLayout m_pipelineLayout = m_device->get().CreatePipelineLayout(&pipelineLayoutDesc);


			//float m_bufferSize = 64 * sizeof(float);

			//wgpu::BufferDescriptor bufferDesc;
			//bufferDesc.mappedAtCreation = false;
			//bufferDesc.size = m_bufferSize;
			//bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
			//wgpu::Buffer inputBuffer = m_device->get().CreateBuffer(&bufferDesc);


			//// Create output buffer: the only difference is the usage
			//bufferDesc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc;
			//wgpu::Buffer outputBuffer = m_device->get().CreateBuffer(&bufferDesc);

			//bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
			//wgpu::Buffer m_mapBuffer = m_device->get().CreateBuffer(&bufferDesc);

			//std::vector<float> input(m_bufferSize / sizeof(float));
			//for (int i = 0; i < input.size(); ++i) {
			//	input[i] = 0.1f * i;
			//}
			//queue.WriteBuffer(inputBuffer, 0, input.data(), m_bufferSize);

			//// Create bind group
			//// Create compute bind group
			//std::vector<wgpu::BindGroupEntry> entries(2);

			//// Input buffer
			//entries[0].binding = 0;
			//entries[0].buffer = inputBuffer;
			//entries[0].offset = 0;
			//entries[0].size = m_bufferSize;

			//// Output buffer
			//entries[1].binding = 1;
			//entries[1].buffer = outputBuffer;
			//entries[1].offset = 0;
			//entries[1].size = m_bufferSize;

			//wgpu::BindGroupDescriptor bindGroupDesc;
			//bindGroupDesc.layout = bindGroupLayout;
			//bindGroupDesc.entryCount = (uint32_t)entries.size();
			//bindGroupDesc.entries = entries.data();
			//wgpu::BindGroup m_bindGroup = m_device->get().CreateBindGroup(&bindGroupDesc);



			//wgpu::CommandEncoderDescriptor encoderDesc{};
			//wgpu::CommandEncoder encoder =
			//	m_device->get().CreateCommandEncoder(&encoderDesc);

			//wgpu::ComputePassDescriptor computePassDesc{};
			//computePassDesc.timestampWrites = nullptr;
			//wgpu::ComputePassEncoder computePass = encoder.BeginComputePass(&computePassDesc);

			//wgpu::ComputePipelineDescriptor computePipelineDesc{};
			//computePipelineDesc.compute.entryPoint = "main";
			//computePipelineDesc.compute.module = computeShadeModule;
			//computePipelineDesc.layout = m_pipelineLayout;

			//wgpu::ComputePipeline computePipeline =
			//	m_device->get().CreateComputePipeline(&computePipelineDesc);

			//computePass.SetPipeline(computePipeline);
			//computePass.SetBindGroup(0, m_bindGroup, 0, nullptr);

			//uint32_t invocationCount = m_bufferSize / sizeof(float);
			//uint32_t workgroupSize = 32;
			//uint32_t workgroupCount = (invocationCount + workgroupSize - 1) / workgroupSize;


			//computePass.DispatchWorkgroups(workgroupCount, 1, 1);
			//computePass.End();

			//encoder.CopyBufferToBuffer(outputBuffer, 0, m_mapBuffer, 0, m_bufferSize);

			//wgpu::CommandBufferDescriptor cmdBufDesc{};
			//cmdBufDesc.label = "Compute Command Buffer";
			//wgpu::CommandBuffer commands = encoder.Finish(&cmdBufDesc);
			//queue.Submit(1, &commands);

			//wgpu::Future f = m_mapBuffer.MapAsync(wgpu::MapMode::Read, 0, m_bufferSize, wgpu::CallbackMode::WaitAnyOnly, [&](wgpu::MapAsyncStatus status, wgpu::StringView message) {
			//	if (status == wgpu::MapAsyncStatus::Success) {
			//		const float* output = (const float*)m_mapBuffer.GetConstMappedRange(0, m_bufferSize);
			//		for (int i = 0; i < input.size(); ++i) {
			//			std::cout << i + 1 << " : input " << input[i] << " became " << output[i] << std::endl;
			//			std::cout << ((input[i] + 1 == output[i]) ? "PASS" : "FAIL") << std::endl;
			//		}
			//		m_mapBuffer.Unmap();
			//	}
			//
			//	});

			///*while (!done)
			//{
			//	instance.ProcessEvents();
			//}*/

			//instance.WaitAny(f, UINT64_MAX);

			//while (true) {
			//	instance.ProcessEvents();
			//}

			////release resources
			//inputBuffer.Destroy();
			//outputBuffer.Destroy();
			//m_mapBuffer.Destroy();
			//m_device->get().Destroy();
	}

	Controller::~Controller() = default;
}