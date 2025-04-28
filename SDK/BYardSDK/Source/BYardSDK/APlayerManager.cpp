#include "pch.h"
#include "APlayerManager.h"
#include "SDKHooks.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AUnitPlayer* APlayerManager::GetPlayer( PLAYER a_ePlayerIndex /*= PLAYER_1 */ )
{
	return CALL_THIS( 0x0061faf0, APlayerManager*, AUnitPlayer*, this, PLAYER, a_ePlayerIndex );
}
