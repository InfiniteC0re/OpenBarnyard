#include "pch.h"
#include "ASteerRequirement.h"
#include "AUnit.h"
#include "Player/APlayerManager.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b0ad0, CreateObject 005b0c90
// NOTE: GetClass 005b0ad0, CreateObject 005b0c90, CreateObjectInPlace 005b0d00
TDEFINE_CLASS( ASteerRequirement );

// $Barnyard: FUNCTION 005b0c20
// $Barnyard: FUNCTION 005b0d70
ASteerRequirement::~ASteerRequirement()
{
}

// $Barnyard: FUNCTION 005b0b60
TBOOL ASteerRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pValue = a_pProperties->GetProperty( "steer" );

	if ( pValue )
		m_strSteer = pValue->GetTPString8();

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b0ae0
void ASteerRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	ASteer* pSteer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer();

	// NOTE: only the weight is set
	if ( TStringManager::String8CompareNoCase( m_strSteer.GetString(), pSteer->GetClass()->GetName() ) == 0 )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}
