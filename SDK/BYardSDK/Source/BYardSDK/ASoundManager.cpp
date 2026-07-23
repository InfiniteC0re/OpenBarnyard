#include "pch.h"
#include "ASoundManager.h"
#include "SDKHooks.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT ASoundManager::PlayCue( TINT a_iCue )
{
	return CALL_THIS( 0x005dade0, ASoundManager*, TINT, this, TINT, a_iCue );
}
