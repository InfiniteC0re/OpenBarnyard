#include "pch.h"
#include "AInputManager2.h"
#include "Platform/Windows/Input/TInputInterface_Win.h"

AInputManager2::AInputManager2()
{
	TIMPLEMENT();

	Toshi::TInputDXInterface::CreateSingleton();
}

AInputManager2::~AInputManager2()
{
	TIMPLEMENT();
}