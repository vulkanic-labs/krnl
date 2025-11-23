#include "core/device.hpp"
#include "core/log.h"
#include <vector>
#include <iostream>


namespace krnl
{

	Device::Device(wgpu::Instance instance)
	{
		wgpu::Adapter adapter;
		wgpu::RequestAdapterOptions options{};
		options.powerPreference = wgpu::PowerPreference::HighPerformance;
		wgpu::Future f1 =
			instance.RequestAdapter(&options, wgpu::CallbackMode::WaitAnyOnly,
				[&adapter](wgpu::RequestAdapterStatus status, wgpu::Adapter a,
					wgpu::StringView message)
				{
					if (status != wgpu::RequestAdapterStatus::Success) {
						KRNL_ERROR("RequestAdapter: " << message);
						exit(0);
					}
					adapter = std::move(a); });

		instance.WaitAny(f1, UINT64_MAX);

		wgpu::DeviceDescriptor desc{};
		desc.SetUncapturedErrorCallback([](const wgpu::Device&,
			wgpu::ErrorType errorType,
			wgpu::StringView message)
			{ 
				KRNL_ERROR("WebGPU Device Error: " << errorType << message);
			});

		desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowProcessEvents,
			[](const wgpu::Device&,
				wgpu::DeviceLostReason reason,
				wgpu::StringView message)
			{
				KRNL_ERROR("WebGPU Device Lost: " << reason << message);
			});


		wgpu::Future f2 = adapter.RequestDevice(
			&desc, wgpu::CallbackMode::WaitAnyOnly,
			[this](wgpu::RequestDeviceStatus status, wgpu::Device d,
				wgpu::StringView message)
			{
				if (status != wgpu::RequestDeviceStatus::Success)
				{
					KRNL_ERROR("RequestDevice: " << message);
					exit(0);
				}
				this->m_device = std::move(d);
			});
		instance.WaitAny(f2, UINT64_MAX);

		KRNL_LOG("WebGPU device created successfully");
	}

} // namespace krnl