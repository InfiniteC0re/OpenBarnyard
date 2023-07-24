#include "pch.h"
#include "AInputHelper.h"

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, TBOOL bVal, float a_fRepeatTime)
{
	Toshi::T2Vector<ButtonInfo, 4> buttonInfoVector;

	if (bVal)
	{
		TASSERT(0.01666f <= a_fRepeatTime);
	}

	AInputManager2* inputMng = AInputManager2::GetSingleton();

	if (m_eInputContext == AInputMap::INPUTCONTEXT_UNK12)
	{
		m_eInputContext = inputMng->GetContext();
	}

	auto buttonMap = inputMng->m_InputMap.GetButtonMap(inputMng->GetContext());
	auto foundMap1 = buttonMap.Find(TSTATICCAST(AInputMap::ActionButton, a_eInputButton));

	if (buttonMap.Begin() != buttonMap.End())
	{
		for (auto j = foundMap1->Begin(); j != foundMap1->End(); j++)
		{
			buttonInfoVector.PushBack(*(ButtonInfo*)j);
		}
	}

	uint32_t buttonDevice = MakeButtonDevice(a_eInputButton, a_eInputDevice);

	m_oButtonMap.Insert(buttonDevice, buttonInfoVector);
}

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, TBOOL bVal, float a_fRepeatTime)
{
	for (size_t i = 0; i < AInputManager2::INPUTDEVICE_Count; i++)
	{
		AddMapping(a_eInputButton, i, bVal, a_fRepeatTime);
	}
	AddMapping(a_eInputButton, AInputManager2::INPUTDEVICE_Keyboard, bVal, a_fRepeatTime);
}

TBOOL AInputHelper::IsJustDown(AInputMap::INPUTBUTTON a_eInputButton)
{
	for (size_t i = 0; i < AInputManager2::INPUTDEVICE_Count; i++)
	{
		if (IsJustDown(a_eInputButton, i))
		{
			return TTRUE;
		}
	}
	return IsJustDown(a_eInputButton, AInputManager2::INPUTDEVICE_Keyboard);
}

TBOOL AInputHelper::IsJustDown(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice)
{
	auto inputMng = AInputManager2::GetSingleton();

	if (a_eInputDevice != AInputManager2::INPUTDEVICE_Keyboard && inputMng->CheckIfValidDevice(inputMng->GetDeviceHandle(a_eInputDevice)))
	{
		return TFALSE;
	}

	for (auto i = m_oButtonMap.Begin(); i != m_oButtonMap.End(); i++)
	{
		auto value = i->GetSecond();
		for (size_t j = 0; j < value.Size(); j++)
		{
			if (HASFLAG(value[j] & 1))
			{
				return TTRUE;
			}
		}
	}

	return TFALSE;

}

void AInputHelper::Update(float fVal)
{
	AInputMap::INPUTBUTTON eInputButton;
	AInputManager2::INPUTDEVICE eInputDevice;

	for (auto i = m_oButtonMap.Begin(); i != m_oButtonMap.End(); i++)
	{
		auto key = i->GetFirst();
		auto value = i->GetSecond();
		GetInputButtonDevice(key, eInputButton, eInputDevice);
		for (size_t i = 0; i < value.Size(); i++)
		{
			UpdateButtonInfo(&value[i], eInputDevice);
		}
	}
}

void AInputHelper::UpdateButtonInfo(ButtonInfo* a_pButtonInfo, AInputManager2::INPUTDEVICE a_eInputDevice)
{

}
