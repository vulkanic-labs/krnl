#pragma once
#include <webgpu/webgpu_cpp.h>
#include <memory>
#include "core/future.hpp"

namespace krnl
{
	struct InstanceImpl;

	class Instance
	{
	public:
		Instance();
		~Instance();

		void ProcessEvents() const { m_Instance.ProcessEvents(); }
		inline void WaitAny(const krnl::Future& future, uint64_t timeoutMs) const
		{
			m_Instance.WaitAny(future.GetNative(), timeoutMs);
		}

		const wgpu::Instance& GetNative() const { return m_Instance; }
	private:
		std::unique_ptr <InstanceImpl> m_Impl;
		wgpu::Instance m_Instance;
	};

} // namespace krnl
