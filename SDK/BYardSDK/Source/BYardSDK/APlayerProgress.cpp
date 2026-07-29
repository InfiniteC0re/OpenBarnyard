#include "pch.h"
#include "APlayerProgress.h"
#include "APlayerProfileManager.h"
#include "AGameStateController.h"
#include "SDKHooks.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void APlayerProgress::ClearActiveSlot()
{
	CALL_THIS( 0x00651a90, APlayerProgress*, void, this );
}

void APlayerProgress::StartNewGame()
{
	APlayerProfileManager::GetSingleton()->ResetMainCharacter();

	CALL( 0x0042ab30, void ); // request a fresh game (DAT_007817ec = 1)

	// SetFlags ORs its argument, so 0x2022 = the retail flags 0x20 | 0x2002
	AGameStateController::GetSingleton()->SetFlags( 0x2022 );
}

void APlayerProgress::LoadGame()
{
	CALL( 0x00423c70, void, char, 0 );
}
