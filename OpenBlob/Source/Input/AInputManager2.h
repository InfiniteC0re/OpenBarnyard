#pragma once
#include "Input/AInputMap.h"

#include <Toshi/Core/TStack.h>

class AInputManager2 : public Toshi::TSingleton<AInputManager2>
{
public:

	enum INPUTDEVICE
	{
		INPUTDEVICE_Unk = 1,
		INPUTDEVICE_Unk2,
		INPUTDEVICE_Count,
		INPUTDEVICE_Unk3,
		INPUTDEVICE_Unk4,
		INPUTDEVICE_Invalid,
	};

	struct AInputDeviceHandle
	{
		void* m_unk; // 0x0
	};

	Toshi::TStack<AInputMap::INPUTCONTEXT, 10> m_inputContextStack; // 0x1A0
	void* m_unk; // 0x180

	bool CheckIfValidDevice(const AInputDeviceHandle& handle)
	{
		return m_unk == 0 ? true : handle.m_unk == m_unk;
	}

	void SetContext(AInputMap::INPUTCONTEXT a_eInputContext)
	{
		m_inputContextStack.Push(a_eInputContext);
	}

	AInputManager2();
	~AInputManager2();
};