#include "pch.h"
#include "AGenderRequirement.h"
#include "AUnitPlayer.h"
#include "Player/APlayerProfileManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ab070, CreateObject 005ab090
// NOTE: GetClass 005ab070, CreateObject 005ab090, CreateObjectInPlace 005ab0f0
TDEFINE_CLASS( AGenderRequirement );

// $Barnyard: FUNCTION 005ab080
// $Barnyard: FUNCTION 005ab150
AGenderRequirement::~AGenderRequirement()
{
}

// $Barnyard: FUNCTION 005ab1f0
TBOOL AGenderRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pValue = a_pProperties->GetProperty( "gender" );

	if ( pValue )
		m_eGender = ( pValue->GetTPString8() == TPS8D( "male" ) ) ? APlayerProfile::GENDER_MALE : APlayerProfile::GENDER_FEMALE;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ab170
void AGenderRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TBOOL bFound = TFALSE;

	for ( TINT i = 0; i < a_pParticipants->iNumUnits; i++ )
	{
		if ( a_pParticipants->Get( i )->GetClass()->IsA( &TGetClass( AUnitPlayer ) ) && APlayerProfileManager::GetSingleton()->GetMainGender() == m_eGender )
		{
			bFound = TTRUE;
			break;
		}
	}

	// NOTE: only the weight is set
	if ( bFound )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}
