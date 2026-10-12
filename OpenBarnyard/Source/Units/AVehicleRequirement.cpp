#include "pch.h"
#include "AVehicleRequirement.h"
#include "Helpers/ABikeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b2210, CreateObject 005b2280
// NOTE: GetClass 005b2210, CreateObject 005b2280, CreateObjectInPlace 005b22e0
TDEFINE_CLASS( AVehicleRequirement );

TPSTRING8_DECLARE( bikeflag );

// $Barnyard: FUNCTION 005b2220
// $Barnyard: FUNCTION 005b2330
AVehicleRequirement::~AVehicleRequirement()
{
}

// $Barnyard: FUNCTION 005b2230
TBOOL AVehicleRequirement::Create( const PBProperties* a_pProperties )
{
	m_eVehicle = VEHICLE_NONE;

	TBOOL bBike = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bBike, TPS8( bikeflag ) );

	if ( bBike )
		m_eVehicle = VEHICLE_BIKE;

	return AInteractionRequirement::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005b2350
void AVehicleRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TBOOL bOnBike = ABikeManager::GetSingleton()->IsPlayerOnBike() == TTRUE;

	TBOOL bAllowed;

	if ( m_eVehicle == VEHICLE_NONE )
		bAllowed = !bOnBike;
	else if ( m_eVehicle == VEHICLE_BIKE )
		bAllowed = bOnBike;
	else
		bAllowed = TFALSE;

	// NOTE: only the weight is set
	if ( bAllowed )
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 0.0f : 1.0f;
	else
		m_oRoles.fWeight = ( m_bInvert == TTRUE ) ? 1.0f : 0.0f;
}
