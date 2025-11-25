#pragma once
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <memory>
#include "krnl_parameter_set.hpp" // ParameterSet header (expects krnl::ParameterSet)
#include "krnl_buffer.hpp"        // Buffer header (for type info if needed)

namespace krnl {

    /**
     * Pipeline (compute)
     *
     * Lightweight wrapper for a compute pipeline.
     * - Builds pipeline layout using the provided ParameterSet
     * - Optionally creates shader module from WGSL source
     * - Provides encodeDispatch(pass, x,y,z) to set pipeline, bind group and dispatch
     */
    class Pipeline {
    public:
        // Build from WGSL text
        static Pipeline CreateComputeFromWGSL(
            wgpu::Device device,
            const std::string& wgslSource,
            ParameterSet& params,
            const std::string& entryPoint = "main",
            const char* label = nullptr
        );

        // Build from an existing shader module
        static Pipeline CreateComputeFromModule(
            wgpu::Device device,
            const wgpu::ShaderModule& module,
            ParameterSet& params,
            const std::string& entryPoint = "main",
            const char* label = nullptr
        );

        ~Pipeline() = default;

        // Encode a dispatch into an existing compute pass encoder.
        // This sets pipeline, bind group 0 (paramSet) and optionally issues push constants (if set).
        void encodeDispatch(wgpu::ComputePassEncoder& pass, uint32_t x, uint32_t y = 1, uint32_t z = 1);

        // Set push-constant data that will be applied on the next encodeDispatch call.
        // If backend doesn't support push constants, user will see a warning; fallback can be implemented.
        void setPushConstants(const void* data, size_t bytes);

        // Accessors
        const wgpu::ComputePipeline& getNative() const { return m_pipeline; }
        const wgpu::PipelineLayout& getLayout() const { return m_pipelineLayout; }

    private:
        Pipeline() = default;

        void buildPipeline(const wgpu::ShaderModule& module, const std::string& entryPoint, const char* label);

    private:
        wgpu::Device m_device;
        wgpu::PipelineLayout m_pipelineLayout;
        wgpu::ComputePipeline m_pipeline;
        wgpu::ShaderModule m_shaderModule;

        // parameter set (non-owning)
        ParameterSet* m_params = nullptr;

        // push constants buffer (if setPushConstants called)
        std::vector<uint8_t> m_pushData;
        bool m_hasPushConstants = false;
    };
} // namespace krnl
