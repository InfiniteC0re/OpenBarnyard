#include "pch.h"
#include "AInputManager2.h"
#include TOSHI_MULTIPLATFORM(Input/TInputInterface)


AInputManager2::AInputManager2()
{
	TIMPLEMENT();

	Toshi::TUtil::MemClear(m_pInputDeviceHandle, sizeof(m_pInputDeviceHandle));

	auto input = new Toshi::TInputDXInterface();
	TBOOL result = input->Initialise();
	TASSERT(result == TTRUE);

	for (size_t i = 0; i < INPUTDEVICE_Invalid; i++)
	{
		input->AcquireAll();
		input->ProcessEvents(0.1f);
		Sleep(200);
	}
}

AInputManager2::~AInputManager2()
{
	TIMPLEMENT();
}