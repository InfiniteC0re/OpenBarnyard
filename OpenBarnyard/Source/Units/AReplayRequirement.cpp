#include "pch.h"
#include "AReplayRequirement.h"
#include "Helpers/AReplayManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005af360, CreateObject 005af480
// NOTE: GetClass 005af360, CreateObject 005af480, CreateObjectInPlace 005af4f0
TDEFINE_CLASS( AReplayRequirement );

TPSTRING8_DECLARE( minigame );

// $Barnyard: FUNCTION 005af3a0
// $Barnyard: FUNCTION 005af560
AReplayRequirement::~AReplayRequirement()
{
}

// $Barnyard: FUNCTION 005af370
TBOOL AReplayRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strMiniGame, TPS8( minigame ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005af410
void AReplayRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TBOOL bUnlocked = AReplayManager::GetSingleton()->IsReplayUnlocked( m_strMiniGame, TTRUE );

	// NOTE: only the weight is set
	if ( bUnlocked )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 2.0f : -1.0f;
}
