#include "pch.h"
#include "AInputManager2.h"
#include TOSHI_MULTIINPUT(TInputInterface)

AInputManager2::AInputManager2()
{
	TIMPLEMENT();

	auto input = Toshi::TInputDXInterface();
	bool result = input.Initialise();
	TASSERT(result == TTRUE);
}

AInputManager2::~AInputManager2()
{
	TIMPLEMENT();
}