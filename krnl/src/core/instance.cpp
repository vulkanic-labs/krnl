#include "core/instance.hpp"
#include "core/device.hpp"
#include "core/log.h"

#include <memory>

namespace krnl
{

    Instance::Instance()
    {
        static const auto kTimedWaitAny = wgpu::InstanceFeatureName::TimedWaitAny;
        wgpu::InstanceDescriptor desc = {.requiredFeatureCount = 1,
                                         .requiredFeatures = &kTimedWaitAny};
        m_Instance = wgpu::CreateInstance(&desc);

        if (!m_Instance)
        {
            KRNL_ERROR("Failed to create GPU instance");
            std::exit(EXIT_FAILURE);
        }

        KRNL_LOG("Instance created successfully");
    }

    Instance::~Instance()
    {
    }

} // namespace krnl
