#include "pch.h"
#include "AUITextResolver.h"

#include <BYardSDK/SDKHooks.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void AUITextResolver::ResolveActionNames( TWCHAR* a_wcsOutBuffer, TUINT a_uiOutMaxSize, const TWCHAR* a_wcsString )
{
	CALL( 0x0042bc80, void, TWCHAR*, a_wcsOutBuffer, TUINT, a_uiOutMaxSize, const TWCHAR*, a_wcsString );
}
