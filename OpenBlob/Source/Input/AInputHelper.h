#pragma once
#include <Input/AInputManager2.h>

class AInputHelper
{
public:
	AInputHelper()
	{
		m_eInputContext = AInputMap::INPUTCONTEXT_UNK12;
	}

	enum ButtonInfo
	{

	};

	void AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, TBOOL bVal, float a_fRepeatTime);
	void AddMapping(AInputMap::INPUTBUTTON a_eInputButton, TBOOL bVal, float a_fRepeatTime);

	void Update(float fVal = 0.0f);

	uint32_t MakeButtonDevice(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eDevice)
	{
		TASSERT(0 == (0xFFFF0000 & a_eInputButton));
		TASSERT(0 == (0xFFFF0000 & a_eDevice));
		return (a_eInputButton << 16 | a_eDevice);
	}

	void GetInputButtonDevice(uint32_t a_iButtonDevice, AInputMap::INPUTBUTTON& a_eInputButton, AInputManager2::INPUTDEVICE& a_eInputDevice)
	{
		a_eInputButton = a_iButtonDevice >> 16;
		a_eInputDevice = a_iButtonDevice & 0xFFFF;
	}

	typedef Toshi::TComparator<int> ButtonMapComparator;

	AInputMap::INPUTCONTEXT m_eInputContext;                                                  // 0x4
	Toshi::T2Map<uint32_t, Toshi::T2Vector<ButtonInfo, 4>, ButtonMapComparator> m_oButtonMap; // 0x8
};

