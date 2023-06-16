#include "pch.h"
#include "AInputManager2.h"
#include TOSHI_MULTIPLATFORM(Input/TInputInterface)

AInputManager2::AInputManager2()
{
	TIMPLEMENT();

	auto input = new Toshi::TInputDXInterface();
	TBOOL result = input->Initialise();
	TASSERT(result == TTRUE);
}

AInputManager2::~AInputManager2()
{
	TIMPLEMENT();
}