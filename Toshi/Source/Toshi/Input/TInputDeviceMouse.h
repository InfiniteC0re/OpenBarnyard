#pragma once

#include "TInputDevice.h"

namespace Toshi
{
	class TInputDeviceMouse : public TInputDevice
	{
	public:
		TInputDeviceMouse() : TInputDevice()
		{

		}
	public:

		virtual bool GetDoodadProperties(int doodad, TInputDevice::DoodadProperties& props) const;
		virtual const char* GetButtonFromDoodad(int doodad) const;

	};
}