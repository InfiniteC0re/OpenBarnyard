#pragma once
#include "Input/AInputMap.h"
#include "AStack.h"

#include <Toshi/Core/Platform.h>
#include <Toshi/Input/TInputInterface.h>

class AInputManager2 :
	public Toshi::TSingleton<AInputManager2>
{
public:
	typedef uint32_t INPUTDEVICE;
	enum INPUTDEVICE_ : INPUTDEVICE
	{
		INPUTDEVICE_Controller1,
		INPUTDEVICE_Controller2,
		INPUTDEVICE_Controller3,
		INPUTDEVICE_Controller4,
		INPUTDEVICE_Count,
		INPUTDEVICE_Keyboard,
		INPUTDEVICE_Invalid,
	};

	class AInputDeviceHandle
	{
	public:
		bool IsValid() const
		{
			return m_pDevice != TNULL;
		}

	public:
		Toshi::TInputDevice* m_pDevice = TNULL; // 0x0
		int m_iId                      = 0;     // 0x4
	};

public:
	AInputManager2();
	~AInputManager2();

	void Update(float a_fDeltaTime);
	void UpdateControllers();

	TBOOL HasDeviceOfPlatform(Toshi::Platform a_ePlatform) const;

	void AddInGameController(const AInputDeviceHandle& a_Handle);
	void RemoveInGameController(const AInputDeviceHandle& a_Handle);

	AInputDeviceHandle GetControllerHandle(INPUTDEVICE a_eDevice);

	TBOOL CheckIfValidDevice(const AInputDeviceHandle& a_Handle) const
	{
		if (m_pSomeDevice == TNULL)
			return TTRUE;

		return a_Handle.m_pDevice == m_pSomeDevice;
	}

	void SetContext(AInputMap::INPUTCONTEXT a_eInputContext)
	{
		m_inputContextStack.Push(a_eInputContext);
	}

	AInputMap::INPUTCONTEXT GetContext()
	{
		return m_inputContextStack.IsEmpty() ? AInputMap::INPUTCONTEXT_UNK12 : m_inputContextStack.Top();
	}

	AInputMap& GetInputMap()
	{
		return m_InputMap;
	}

private:
	AInputDeviceHandle m_aControllers[4];                         // 0x04
	Toshi::T2Vector<AInputDeviceHandle, 2> m_pInputDeviceHandles;
	AInputMap m_InputMap;                                         // 0x048
	Toshi::TInputDevice* m_pSomeDevice;                           // 0x180
	AStack<AInputMap::INPUTCONTEXT, 10> m_inputContextStack;      // 0x1A0
};