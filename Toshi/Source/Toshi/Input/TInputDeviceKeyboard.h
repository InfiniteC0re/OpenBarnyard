#pragma once
#include "TInputDevice.h"

namespace Toshi
{
	class TInputDeviceKeyboard : public TInputDevice
	{
		
	public:

		virtual bool GetDoodadProperties(int doodad, DoodadProperties& doodadProps) const
		{
			if (doodad >= 0x20000 && doodad < 0x20080)
			{
				doodadProps.m_iUnk = 0;
				doodadProps.m_iUnk2 = 0;
				return true;
			}
			return false;
		}

		virtual bool GetButtonFromDoodad(int doodad)
		{
			switch (doodad)
			{
			case 0x20002:
				return "VIRTKEY_SHIFT";
			case 0x20003:
				return "VIRTKEY_CONTROL";
			case 0x20004:
				return "VIRTKEY_ALT";
			case 0x20005:
				return "KEY_ESCAPE";
			case 0x20006:
				return "KEY_1";
			case 0x20007:
				return "KEY_2";
			case 0x20008:
				return "KEY_3";
			case 0x20009:
				return "KEY_4";
			case 0x2000A:
				return "KEY_5";
			case 0x2000B:
				return "KEY_6";
			case 0x2000C:
				return "KEY_7";
			case 0x2000D:
				return "KEY_8";
			case 0x2000E:
				return "KEY_9";
			case 0x2000F:
				return "KEY_0";
			default:
				return "UNKNOWN OR UNIMPLEMENTED";
			}
		}

		virtual int GetAxisCount() { return 0; }

	};
}