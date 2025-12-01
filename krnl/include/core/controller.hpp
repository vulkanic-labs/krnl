#pragma once
#include <memory>
#include "core/instance.hpp" 
#include "core/device.hpp"

namespace krnl
{
	class Controller
	{
	public:
		Controller();
		~Controller();

	private:
		Instance m_instance;
		Device m_device;
	};
}

