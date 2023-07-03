#include "pch.h"
#include "AInputManager2.h"
#include TOSHI_MULTIPLATFORM(Input/TInputInterface)


void AInputManager2::AddInGameController(const AInputDeviceHandle& a_Handle)
{
	TASSERT(a_Handle.IsValid());

	/*if (m_pInputDeviceHandles.Find(a_Handle) != m_pInputDeviceHandles.End())
	{
		m_pInputDeviceHandles.PushBack(a_Handle);
	}*/
}

void AInputManager2::RemoveInGameController(const AInputDeviceHandle& a_Handle)
{
	TASSERT(a_Handle.IsValid());

	//if (m_pInputDeviceHandles.Find(a_Handle) != m_pInputDeviceHandles.End())
	//{
	//	//m_pInputDeviceHandles.Erase(a_Handle);
	//}
}

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