#include "pch.h"
#include "Input/AInputHandler.h"

#include <Toshi/Input/TInputInterface.h>
#include <Platform/Windows/Input/TInputInterface_Win.h>

AInputHandler::AInputHandler()
{
	TIMPLEMENT();

	auto pInputInterfaceClass = Toshi::TClass::Find("TInputDXInterface");
	auto pInputInterface = pInputInterfaceClass->CreateObject();
	TASSERT(pInputInterface && pInputInterface->IsA(&TGetClass(Toshi::TInputInterface)));
}

AInputHandler::~AInputHandler()
{
	if (Toshi::TInputInterface::GetSingleton())
	{
		delete Toshi::TInputInterface::GetSingleton();
	}
}

TBOOL AInputHandler::OnCreate()
{
	TIMPLEMENT();
	Toshi::TInputInterface::GetSingleton()->Initialise();
	m_pMouseDevice = Toshi::TInputInterface::GetSingleton()->GetDeviceByIndex<Toshi::TInputDeviceMouse>(0);

	return TTRUE;
}

TBOOL AInputHandler::OnUpdate(TFLOAT a_fDeltaTime)
{
	TIMPLEMENT();
	Toshi::TInputInterface::GetSingleton()->AcquireAll();
	Toshi::TInputInterface::GetSingleton()->ProcessEvents(a_fDeltaTime);
	return TTRUE;
}
