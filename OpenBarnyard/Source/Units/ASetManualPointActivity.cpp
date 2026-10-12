#include "pch.h"
#include "ASetManualPointActivity.h"
#include "AWorldPositionActivityMessage.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005bd2b0, CreateObject 005bd2d0, CreateObjectInPlace 005bd340
TDEFINE_CLASS( ASetManualPointActivity );

// NOTE: inlined in CreateObject
ASetManualPointActivity::ASetManualPointActivity()
    : m_ePositionType( POSITIONTYPE_UNIT )
{
}

// $Barnyard: FUNCTION 005bd2c0
// $Barnyard: FUNCTION 005bd3b0
ASetManualPointActivity::~ASetManualPointActivity()
{
}

// $Barnyard: FUNCTION 005bd3d0
TBOOL ASetManualPointActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strPositionType;

	if ( a_pProperties->GetOptionalPropertyValue( strPositionType, TPS8D( "positiontype" ) ) )
	{
		if ( TStringManager::String8CompareNoCase( strPositionType.GetString(), "unit" ) == 0 )
			m_ePositionType = POSITIONTYPE_UNIT;
		else if ( TStringManager::String8CompareNoCase( strPositionType.GetString(), "goalobject" ) == 0 )
			m_ePositionType = POSITIONTYPE_GOALOBJECT;
	}

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005bd560
AInteractionActivity::RESULT ASetManualPointActivity::Update( AInteractionSequence* a_pSequence )
{
	AUnit* pUnit = a_pSequence->GetUnit( 0 );

	// NOTE: retail leaves these uninitialised for other position types
	TVector3 vecPosition;
	TVector3 vecDirection;

	if ( m_ePositionType == POSITIONTYPE_UNIT || m_ePositionType == POSITIONTYPE_GOALOBJECT )
	{
		pUnit->GetSteer()->CopyPosition3( vecPosition );
		pUnit->GetSteer()->CopyDirection3( vecDirection );
	}

	AWorldPositionActivityMessage* pMessage = a_pSequence->GetActivityMessage<AWorldPositionActivityMessage>( TPS8D( "manualpoint" ) );
	pMessage->SetPosition( vecPosition );
	pMessage->SetDirection( vecDirection );

	return RESULT_DONE;
}
