#pragma once
#include <Input/AInputManager2.h>

class AInputHelper
{

	enum ButtonInfo
	{

	};

	void AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, bool bVal, float a_fRepeatTime);
	void AddMapping(AInputMap::INPUTBUTTON a_eInputButton, bool bVal, float a_fRepeatTime);

	uint32_t MakeButtonDevice(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eDevice)
	{
		TASSERT(0 == (0xFFFF0000 & a_eInputButton));
		TASSERT(0 == (0xFFFF0000 & a_eDevice));
		return (a_eInputButton << 16 | a_eDevice);
	}

	AInputMap::INPUTCONTEXT m_eInputContext; // 0x4
};

