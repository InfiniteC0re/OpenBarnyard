#pragma once
#include "Input/AInputMap.h"

#include "AStack.h"

class AInputManager2 : public Toshi::TSingleton<AInputManager2>
{
public:
	typedef uint32_t INPUTDEVICE;
	enum INPUTDEVICE_ : INPUTDEVICE
	{
		INPUTDEVICE_Unk,
		INPUTDEVICE_Unk1,
		INPUTDEVICE_Unk2,
		INPUTDEVICE_Unk3,
		INPUTDEVICE_Count,
		INPUTDEVICE_Unk4,
		INPUTDEVICE_Unk5,
		INPUTDEVICE_Invalid,
	};

	struct AInputDeviceHandle
	{
		void* m_unk; // 0x0
	};

	AInputMap m_InputMap;                                    // 0x48
	void* m_unk;                                             // 0x180
	AStack<AInputMap::INPUTCONTEXT, 10> m_inputContextStack; // 0x1A0


	TBOOL CheckIfValidDevice(const AInputDeviceHandle& handle) const
	{
		return m_unk == 0 ? TTRUE : handle.m_unk == m_unk;
	}

	void SetContext(AInputMap::INPUTCONTEXT a_eInputContext)
	{
		m_inputContextStack.Push(a_eInputContext);
	}

	AInputMap::INPUTCONTEXT GetContext()
	{
		return m_inputContextStack.IsEmpty() ? AInputMap::INPUTCONTEXT_UNK12 : m_inputContextStack.Top();
	}

	AInputManager2();
	~AInputManager2();
};