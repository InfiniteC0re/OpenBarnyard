#include "pch.h"
#include "ASquirtRequirement.h"
#include "Player/APlayerProfileManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b06d0, CreateObject 005b0720
// NOTE: GetClass 005b06d0, CreateObject 005b0720, CreateObjectInPlace 005b0780
TDEFINE_CLASS( ASquirtRequirement );

TPSTRING8_DECLARE( numsquirted );

// $Barnyard: FUNCTION 005b06e0
// $Barnyard: FUNCTION 005b07d0
ASquirtRequirement::~ASquirtRequirement()
{
}

// $Barnyard: FUNCTION 005b06f0
TBOOL ASquirtRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_iNumSquirted, TPS8( numsquirted ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b07f0
void ASquirtRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	// NOTE: only the weight is set
	if ( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetNumSquirtedUnits() < m_iNumSquirted )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
}
