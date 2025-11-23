#include "core/shader.hpp"

#include <fstream>

namespace krnl
{

	wgpu::ShaderModule loadWGSL(wgpu::Device device, const std::string& source)
	{
		wgpu::ShaderSourceWGSL wgsl{ {.code = source.c_str()} };
		wgpu::ShaderModuleDescriptor shaderModuleDescriptor{ .nextInChain = &wgsl };
		wgpu::ShaderModule shaderModule =
			device.CreateShaderModule(&shaderModuleDescriptor);

		return shaderModule;
	}

	wgpu::ShaderModule readWGSL(wgpu::Device device, std::filesystem::path path)
	{
		std::ifstream file(path);
		std::string src((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		return loadWGSL(device, src);
	}

} // namespace krnl
