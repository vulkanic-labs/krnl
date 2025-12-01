#pragma once
#include <webgpu/webgpu_cpp.h>

namespace krnl {
	class Future {
	public:
		Future() = default;
		Future(const wgpu::Future& f): m_Future(f) {}
		const wgpu::Future& GetNative() const { return m_Future;}

	private:
		wgpu::Future m_Future;
	};
} // namespace krnl
