#include "pch.h"
#include "ARootTask.h"
#include "SDKHooks.h"

void ARootTask::SetPaused( TBOOL a_bPaused )
{
	CALL_THIS( 0x00402e00, ARootTask*, TBOOL, this, TBOOL, a_bPaused );
}
