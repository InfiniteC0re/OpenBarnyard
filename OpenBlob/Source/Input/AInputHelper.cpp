#include "pch.h"
#include "AInputHelper.h"

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, AInputManager2::INPUTDEVICE a_eInputDevice, bool bVal, float a_fRepeatTime)
{
	Toshi::T2Map<AInputMap::ActionButton, Toshi::T2Vector<int, 4>, AInputMap::ButtonMapComparator> map1;
	Toshi::T2Map<AInputMap::ActionButton, Toshi::T2Vector<int, 4>, AInputMap::ButtonMapComparator> map2;
	Toshi::T2Vector<int, 4> input;
	Toshi::T2Vector<ButtonInfo, 4> buttonInfoVector;

	TASSERT(0.01666f <= a_fRepeatTime);

	AInputManager2* inputMng = AInputManager2::GetSingletonWeak();

	if (m_eInputContext == AInputMap::INPUTCONTEXT_UNK12)
	{
		m_eInputContext = inputMng->GetContext();
	}

	auto buttonMap = inputMng->m_InputMap.GetButtonMap(inputMng->GetContext());
	auto foundMap1 = map1.Find((AInputMap::ActionButton)a_eInputButton);
	auto mapEnd2 = map2.End();

	if (foundMap1 != mapEnd2)
	{
		for (auto i = input.Begin(); i != input.End(); i++)
		{
			TWIP();
			buttonInfoVector.PushBack(*(ButtonInfo*)i);
		}
	}

	uint32_t buttonDevice = MakeButtonDevice(a_eInputButton, a_eInputDevice);

	m_oButtonMap.Insert(buttonDevice, buttonInfoVector);
}

void AInputHelper::AddMapping(AInputMap::INPUTBUTTON a_eInputButton, bool bVal, float a_fRepeatTime)
{
	for (size_t i = 0; i < AInputManager2::INPUTDEVICE_Count; i++)
	{
		AddMapping(a_eInputButton, (AInputManager2::INPUTDEVICE)i, bVal, a_fRepeatTime);
	}
	AddMapping(a_eInputButton, AInputManager2::INPUTDEVICE_Unk4, bVal, a_fRepeatTime);
}

void AInputHelper::Update(float fVal)
{
	for (auto i = m_oButtonMap.Begin(); i != m_oButtonMap.End(); i++)
	{
		
	}
}
