#include "core/context.hpp"
#include "core/device.hpp"
#include "core/log.h"

#include <memory>

namespace krnl
{

    Context::Context()
    {
        static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        wgpu::InstanceDescriptor desc = {.requiredFeatureCount = 1,
                                         .requiredFeatures = &kTimedWaitAny};
        m_Instance = wgpu::CreateInstance(&desc);

        if (!m_Instance)
        {
            KRNL_ERROR("Failed to create WebGPU instance");
            std::exit(EXIT_FAILURE);
        }

        KRNL_LOG("WebGPU instance created successfully");
    }

    Context::~Context()
    {
    }

} // namespace krnl
