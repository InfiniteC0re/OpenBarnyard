#pragma once
#include "TInputInterface.h"

namespace Toshi
{
	TOBJECT(TInputDeviceMouse, TInputDevice, TFALSE)
	{
	public:
		static constexpr TINT BUTTON_1 = 0x30001;
		static constexpr TINT BUTTON_2 = 0x30002;
		static constexpr TINT BUTTON_3 = 0x30003;
		static constexpr TINT BUTTON_4 = 0x30004;
		static constexpr TINT BUTTON_5 = 0x30005;
		static constexpr TINT BUTTON_6 = 0x30006;
		static constexpr TINT BUTTON_7 = 0x30007;
		static constexpr TINT BUTTON_8 = 0x30008;
		static constexpr TINT BUTTON_WHEEL_FORWARD = 0x30009;
		static constexpr TINT BUTTON_WHEEL_BACKWARD = 0x3000A;
		static constexpr TINT AXIS_CURSOR = 0x3000B;
		static constexpr TINT AXIS_WHEEL = 0x3000C;

	public:
		TInputDeviceMouse() = default;
		~TInputDeviceMouse() = default;

		virtual TBOOL GetDoodadProperties(Doodad a_iDoodad, TInputDevice::DoodadProperties& props) const override;
		virtual const char* GetButtonFromDoodad(Doodad a_iDoodad) const override;
	};
}