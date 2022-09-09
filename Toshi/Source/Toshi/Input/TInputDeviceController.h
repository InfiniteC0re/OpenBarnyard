#pragma once

#include "Toshi/Input/TInputDevice.h"

namespace Toshi
{
	class TInputDeviceController : TInputDevice
	{
		uint8_t* m_things; // 0x3C
		uint8_t* m_things2; // 0x40

	public:
		TInputDeviceController() : TInputDevice()
		{
			m_things = (uint8_t*)tmalloc(8);
			m_things2 = (uint8_t*)tmalloc(8);
			m_things = { 0 };
			m_things2 = { 0 };
		}
	};
}
