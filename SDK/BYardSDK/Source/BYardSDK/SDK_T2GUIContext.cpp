#include "pch.h"
#include "AGUI2.h"
#include "SDK_T2GUIContext.h"
#include "SDKHooks.h"

SDK_T2GUIContext::SDK_T2GUIContext()
{
	CALL_THIS( 0x006c4700, SDK_T2GUIContext*, SDK_T2GUIContext*, this );
}

void SDK_T2GUIContext::Tick( TFLOAT a_fDeltaTime )
{
	CALL_THIS( 0x006c47f0, SDK_T2GUIContext*, SDK_T2GUIContext*, this, TFLOAT, a_fDeltaTime );
}

void SDK_T2GUIContext::Render()
{
	CALL_THIS( 0x006c4820, SDK_T2GUIContext*, SDK_T2GUIContext*, this );
}
