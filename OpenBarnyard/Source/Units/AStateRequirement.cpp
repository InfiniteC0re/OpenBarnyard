#include "pch.h"
#include "AStateRequirement.h"
#include "GameInterface/AGameStateController.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b0840, CreateObject 005b09d0
// NOTE: GetClass 005b0840, CreateObject 005b09d0, CreateObjectInPlace 005b0a40
TDEFINE_CLASS( AStateRequirement );

// $Barnyard: FUNCTION 005b08d0
// $Barnyard: FUNCTION 005b0ab0
AStateRequirement::~AStateRequirement()
{
}

// $Barnyard: FUNCTION 005b0940
TBOOL AStateRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalPropertyValue( m_strState, TPS8D( "state" ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b0850
void AStateRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	AGameState* pState   = AGameStateController::GetSingleton()->GetCurrentState();
	TBOOL       bAllowed = TStringManager::String8CompareNoCase( pState->GetClass()->GetName(), m_strState.GetString() ) == 0;

	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
		SetUnitAllowed( m_eCategory, i, bAllowed );
}
