#pragma once
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <filesystem>

namespace krnl
{
     wgpu::ShaderModule loadWGSL(wgpu::Device device, const std::string& source);
     wgpu::ShaderModule readWGSL(wgpu::Device device, std::filesystem::path path);
} // namespace krnl
