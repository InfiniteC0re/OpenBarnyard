#pragma once

#include "TInputDevice.h"

namespace Toshi
{
	class TInputDeviceMouse : public TInputDevice
	{
	public:

		static const int BUTTON_1 = 0x30001;
		static const int BUTTON_2 = 0x30002;
		static const int BUTTON_3 = 0x30003;
		static const int BUTTON_4 = 0x30004;
		static const int BUTTON_5 = 0x30005;
		static const int BUTTON_6 = 0x30006;
		static const int BUTTON_7 = 0x30007;
		static const int BUTTON_8 = 0x30008;
		static const int BUTTON_WHEEL_FORWARD = 0x30009;
		static const int BUTTON_WHEEL_BACKWARD = 0x3000A;
		static const int AXIS_CURSOR = 0x3000B;
		static const int AXIS_WHEEL = 0x3000C;

		TInputDeviceMouse() : TInputDevice()
		{

		}
	public:

		virtual bool GetDoodadProperties(int doodad, TInputDevice::DoodadProperties& props) const;
		virtual const char* GetButtonFromDoodad(int doodad) const;

	};
}