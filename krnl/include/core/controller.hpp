#pragma once
#include <memory>
#include "core/context.hpp" 
#include "core/device.hpp"

namespace krnl
{
	class Controller
	{
	public:
		Controller();
		~Controller();

	private:
		Context m_context;
		Device m_device;
	};
}

