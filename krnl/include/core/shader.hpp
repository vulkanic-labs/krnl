#pragma once
#include <webgpu/webgpu_cpp.h>
#include "core/device.hpp"
#include <string>
#include <filesystem>

namespace krnl
{
	class Shader {
	public:

		Shader() = delete;
		Shader(wgpu::ShaderModule shaderModule)
			: m_ShaderModule(shaderModule)
		{
		}

		// Load WGSL source code into a shader module
		static Shader loadWGSL(const Device& device, const std::string& source);
		// Read WGSL source code from a file and load into a shader module
		static Shader readWGSL(const Device& device, std::filesystem::path path);

		const wgpu::ShaderModule& GetNative() const { return m_ShaderModule; }

	private:
		wgpu::ShaderModule m_ShaderModule;
	};
} // namespace krnl
