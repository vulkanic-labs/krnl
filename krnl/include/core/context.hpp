#pragma once
#include <webgpu/webgpu_cpp.h>

namespace krnl
{
	class Context
	{
	public:
		Context();
		~Context();

		wgpu::Instance getInstance() const { return m_Instance; }
	private:
		wgpu::Instance m_Instance;
	};

} // namespace krnl
