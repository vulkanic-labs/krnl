#include "core/device.hpp"
#include "core/log.h"

namespace krnl
{

	Device::Device(const Instance& instance)
	{
		wgpu::Adapter adapter;
		wgpu::RequestAdapterOptions options{};
		options.powerPreference = wgpu::PowerPreference::HighPerformance;
		wgpu::Future f1 =
			instance.GetNative().RequestAdapter(&options, wgpu::CallbackMode::WaitAnyOnly,
				[&adapter](wgpu::RequestAdapterStatus status, wgpu::Adapter a,
					wgpu::StringView message)
				{
					if (status != wgpu::RequestAdapterStatus::Success) {
						KRNL_ERROR("RequestAdapter: " << message);
						exit(0);
					}
					adapter = std::move(a); });

		instance.WaitAny(f1, UINT64_MAX);

		wgpu::AdapterInfo adapterInfo;
		adapter.GetInfo(&adapterInfo);

		KRNL_LOG("GPU Adapter Info:");
		KRNL_LOG("  Vendor: " << adapterInfo.vendor);
		KRNL_LOG("  Architecture: " << adapterInfo.architecture);
		KRNL_LOG("  Device: " << adapterInfo.device);
		KRNL_LOG("  Description: " << adapterInfo.description);
		KRNL_LOG("  Backend: " << adapterInfo.backendType);

		wgpu::DeviceDescriptor desc{};
		desc.SetUncapturedErrorCallback([](const wgpu::Device&,
			wgpu::ErrorType errorType,
			wgpu::StringView message)
			{
				KRNL_ERROR("Device Error: " << errorType << message);
			});

		desc.SetDeviceLostCallback(wgpu::CallbackMode::AllowProcessEvents,
			[](const wgpu::Device&,
				wgpu::DeviceLostReason reason,
				wgpu::StringView message)
			{
				KRNL_ERROR("Device Lost: " << reason << message);
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

		KRNL_LOG("Device acquired successfully");
	}

} // namespace krnl