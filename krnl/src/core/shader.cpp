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

	Shader Shader::loadWGSL(const Device& device, const std::string& source)
	{
		wgpu::ShaderModule shaderModule = krnl::loadWGSL(device.GetNative(), source);
		return Shader(shaderModule);
	}

	Shader Shader::readWGSL(const Device& device, std::filesystem::path path)
	{
		wgpu::ShaderModule shaderModule = krnl::readWGSL(device.GetNative(), path);
		return Shader(shaderModule);
	}

	

} // namespace krnl
