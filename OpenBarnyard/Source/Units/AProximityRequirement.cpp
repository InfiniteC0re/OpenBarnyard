#include "pch.h"
#include "AProximityRequirement.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ae430, CreateObject 005ae550
// NOTE: GetClass 005ae430, CreateObject 005ae550, CreateObjectInPlace 005ae5b0
TDEFINE_CLASS( AProximityRequirement );

// $Barnyard: FUNCTION 005ae440
// $Barnyard: FUNCTION 005ae610
AProximityRequirement::~AProximityRequirement()
{
}

// $Barnyard: FUNCTION 005ae450
TBOOL AProximityRequirement::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pMinDistance = a_pProperties->GetProperty( "mindistance" );

	if ( !pMinDistance )
		return TFALSE;

	m_fMinDistance = ( pMinDistance->GetType() == PBPropertyValue::Type::Float ) ? pMinDistance->GetFloat() : TFLOAT( pMinDistance->GetInteger() );

	const PBPropertyValue* pMaxDistance = a_pProperties->GetProperty( "maxdistance" );

	if ( !pMaxDistance )
		return TFALSE;

	m_fMaxDistance = ( pMaxDistance->GetType() == PBPropertyValue::Type::Float ) ? pMaxDistance->GetFloat() : TFLOAT( pMaxDistance->GetInteger() );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005ae4c0
void AProximityRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TFLOAT fWeight = 1.0f;

	if ( a_pParticipants->iNumUnits > 1 )
	{
		const TVector4* pPosition1 = a_pParticipants->Get( 1 )->GetSteer()->GetPosition();
		const TVector4* pPosition0 = a_pParticipants->Get( 0 )->GetSteer()->GetPosition();
		TFLOAT          fDistance  = TVector4::Distance( *pPosition0, *pPosition1 );

		if ( fDistance < m_fMinDistance || fDistance > m_fMaxDistance )
			fWeight = 0.0f;
	}

	// NOTE: only the weight is set
	if ( m_bInvert == TTRUE )
		fWeight = 1.0f - fWeight;

	m_oRoles.fWeight = fWeight;
}
