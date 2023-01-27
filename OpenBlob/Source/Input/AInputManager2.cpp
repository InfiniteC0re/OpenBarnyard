#include "pch.h"
#include "AInputManager2.h"
#include TOSHI_MULTIRENDER(TInputInterface)

AInputManager2::AInputManager2()
{
	TIMPLEMENT();

	Toshi::TInputDXInterface::CreateSingleton();
}

AInputManager2::~AInputManager2()
{
	TIMPLEMENT();
}