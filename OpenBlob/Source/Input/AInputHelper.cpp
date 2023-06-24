#include "pch.h"
#include "AInputHelper.h"

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, TBOOL bVal, float a_fRepeatTime)
{
	Toshi::T2Map<AInputMap::ActionButton, Toshi::T2Vector<int, 4>, AInputMap::ButtonMapComparator> map1;
	Toshi::T2Map<AInputMap::ActionButton, Toshi::T2Vector<int, 4>, AInputMap::ButtonMapComparator> map2;
	Toshi::T2Vector<int, 4> input;
	Toshi::T2Vector<ButtonInfo, 4> buttonInfoVector;

	if (bVal)
	{
		TASSERT(0.01666f <= a_fRepeatTime);
	}

	AInputManager2* inputMng = AInputManager2::GetSingletonWeak();

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
	AddMapping(a_eInputButton, AInputManager2::INPUTDEVICE_Unk4, bVal, a_fRepeatTime);
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
