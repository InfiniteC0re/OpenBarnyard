#include "pch.h"
#include "AMinigamePlayerAi.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0046cb60, CreateObject 0046cbc0, CreateObjectInPlace 0046cc30 (the constructor is inlined in both)
TDEFINE_CLASS( AMinigamePlayerAi );

// $Barnyard: FUNCTION 0046cb70
AMinigamePlayerAi::AMinigamePlayerAi()
{
}

// $Barnyard: FUNCTION 0046cbb0
// $Barnyard: FUNCTION 0046cc80
AMinigamePlayerAi::~AMinigamePlayerAi()
{
}

// $Barnyard: FUNCTION 0046cb90
void AMinigamePlayerAi::Init( TINT a_iFirstSlot, TINT a_iCount )
{
	m_iEndSlot    = a_iFirstSlot + a_iCount;
	m_iNumPlayers = a_iCount;
}
