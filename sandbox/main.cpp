#include <krnl.hpp>
#include <iostream>

int main()
{
	krnl::Instance instance = krnl::Instance();
	krnl::Device device = krnl::Device(instance);

	krnl::Shader computeShader = krnl::Shader::loadWGSL(device, R"(
			@group(0) @binding(0) var<storage,read> inputBuffer: array<f32,64>;
			@group(0) @binding(1) var<storage,read_write> outputBuffer: array<f32,64>;
			@compute @workgroup_size(32)
			fn main(@builtin(global_invocation_id) id: vec3<u32>) {
				// Apply the function f to the buffer element at index id.x:
				outputBuffer[id.x] = inputBuffer[id.x] + 1;
			}
		)");

	size_t bufferSize = 64 * sizeof(float);
	krnl::Buffer input(device, bufferSize, krnl::BufferUsageType::Storage | krnl::BufferUsageType::CopyDst, "input");
	krnl::Buffer output(device, bufferSize, krnl::BufferUsageType::Storage | krnl::BufferUsageType::CopySrc, "output");
	krnl::Buffer map(device, bufferSize, krnl::BufferUsageType::CopyDst | krnl::BufferUsageType::MapRead, "map");

	std::vector<krnl::ParameterSet::Entry> paramEntries;
	paramEntries.push_back({ input,  krnl::BufferBindingType::ReadOnlyStorage });
	paramEntries.push_back({ output, krnl::BufferBindingType::Storage });
	krnl::ParameterSet params(device, paramEntries);

	auto pipeline = krnl::Pipeline::CreateCompute(
		device,
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

	krnl::CommandList cmd(device);
	cmd.BeginComputePass();
	uint32_t workgroups = (64 + 31) / 32;
	pipeline.encodeDispatch(cmd, workgroups);
	cmd.EndComputePass();

	cmd.CopyBufferToBuffer(output, map, bufferSize);

	cmd.Submit();

	std::vector<float> outputData(bufferSize / sizeof(float), 0.0f);
	krnl::Future f = map.MapAsync(krnl::MapMode::Read, 0, bufferSize, outputData.data());
	instance.WaitAny(f, UINT64_MAX);

	for (size_t i = 0; i < inputData.size(); ++i)
		std::cout << i + 1 << " : input " << inputData[i] << " became " << outputData[i] << std::endl;

	while (true)
	{
		instance.ProcessEvents();
	}
	
    return 0;
}
