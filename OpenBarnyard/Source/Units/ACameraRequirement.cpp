#include "pch.h"
#include "ACameraRequirement.h"
#include "AUnit.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a8650, CreateObject 005a8670, CreateObjectInPlace 005a86e0
TDEFINE_CLASS( ACameraRequirement );

TPSTRING8_DECLARE( type );

// $Barnyard: FUNCTION 005a8660
// $Barnyard: FUNCTION 005a8740
ACameraRequirement::~ACameraRequirement()
{
}

// $Barnyard: FUNCTION 005a87f0
TBOOL ACameraRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strType;
	a_pProperties->GetOptionalPropertyValue( strType, TPS8( type ) );

	if ( strType == TPS8D( "inview" ) )
		m_eType = TYPE_INVIEW;

	a_pProperties->GetOptionalFloatOrIntValue( m_fViewDistance, TPS8D( "viewdistance" ) );

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005a8910
void ACameraRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	if ( m_eType == TYPE_INVIEW )
	{
		EvaluateInView( a_pParticipants );
		return;
	}

	// NOTE: only the weight is set
	m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}

// $Barnyard: FUNCTION 005a8760
void ACameraRequirement::EvaluateInView( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	for ( TINT i = 0; i < iNumUnits; i++ )
	{
		AUnit* pUnit    = a_pParticipants->Get( i );
		TBOOL  bAllowed = TFALSE;

		if ( pUnit )
		{
			TVector4 vecPosition = *pUnit->GetSteer()->GetPosition();
			bAllowed             = ACameraManager::GetSingleton()->GetCurrentCamera()->IsPointInView( vecPosition, m_fViewDistance );
		}

		SetUnitAllowed( m_eCategory, i, bAllowed );
	}
}
