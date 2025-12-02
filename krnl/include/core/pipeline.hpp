#pragma once
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <memory>
#include "core/parameterset.hpp" 
#include "core/buffer.hpp"   
#include "core/device.hpp"
#include "core/shader.hpp"
#include "core/commandlist.hpp"

namespace krnl {

    class Pipeline {
    public:
        static Pipeline CreateCompute(
            const Device& device,
            const Shader& module,
            const ParameterSet& params,
            const std::string& entryPoint = "main",
            const char* label = nullptr
        );

        ~Pipeline() = default;

        void encodeDispatch(const CommandList& cmd, uint32_t x, uint32_t y = 1, uint32_t z = 1);

        const wgpu::ComputePipeline& getNative() const { return m_Pipeline; }
        const wgpu::PipelineLayout& getLayout() const { return m_PipelineLayout; }

    private:
        Pipeline() = default;
        Pipeline(const Device& device,const ParameterSet& params)
            : m_Device(device), m_Params(params)
        {
		}

        void buildPipeline(const wgpu::ShaderModule& module, const std::string& entryPoint, const char* label);

    private:
        const Device& m_Device;
        const ParameterSet& m_Params;

        wgpu::PipelineLayout m_PipelineLayout;
        wgpu::ComputePipeline m_Pipeline;
        wgpu::ShaderModule m_ShaderModule;
    };
} // namespace krnl
