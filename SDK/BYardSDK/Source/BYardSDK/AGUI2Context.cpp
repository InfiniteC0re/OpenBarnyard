#include "pch.h"
#include "AGUI2.h"
#include "AGUI2Context.h"
#include "SDKHooks.h"

AGUI2Context::AGUI2Context()
{
	CALL_THIS(0x006c4700, AGUI2Context*, AGUI2Context*, this);
}

void AGUI2Context::Tick(TFLOAT a_fDeltaTime)
{
	CALL_THIS(0x006c47f0, AGUI2Context*, AGUI2Context*, this, TFLOAT, a_fDeltaTime);
}

void AGUI2Context::Render()
{
	CALL_THIS(0x006c4820, AGUI2Context*, AGUI2Context*, this);
}
