#include "pch.h"
#include "ASteerActivity.h"
#include "AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c1b00, CreateObject 005c1b20
// NOTE: GetClass 005c1b00, CreateObject 005c1b20, CreateObjectInPlace 005c1ba0
TDEFINE_CLASS( ASteerActivity );

ASteerActivity::ASteerActivity()
    : m_eType( TYPE_ALWAYS )
    , m_pSteerClass( TNULL )
{
}

// $Barnyard: FUNCTION 005c1b10
// $Barnyard: FUNCTION 005c1c10
ASteerActivity::~ASteerActivity()
{
}

// $Barnyard: FUNCTION 005c1cb0
TBOOL ASteerActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strSteer;
	TPString8 strType;

	m_eType = TYPE_ALWAYS;

	a_pProperties->GetOptionalPropertyValue( strType, TPS8D( "type" ) );

	if ( strType == TPS8D( "whennetwork" ) )
		m_eType = TYPE_WHEN_NETWORK;

	a_pProperties->GetOptionalPropertyValue( strSteer, TPS8D( "steer" ) );
	m_pSteerClass = const_cast<TClass*>( TClass::Find( strSteer.GetString(), &TGetClass( ASteer ) ) );

	return AInteractionActivity::Create( a_pProperties );
}

// $Barnyard: FUNCTION 005c1c30
AInteractionActivity::RESULT ASteerActivity::Update( AInteractionSequence* a_pSequence )
{
	AUnit* pUnit = a_pSequence->GetUnitByCategory( m_eCategory );

	if ( m_eType == TYPE_ALWAYS )
	{
		if ( pUnit )
			pUnit->ChangeSteer( m_pSteerClass );
	}
	else if ( m_eType == TYPE_WHEN_NETWORK && pUnit )
	{
		AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

		// Only change the steer if the unit is inside of the network
		if ( pNetwork && pNetwork->FindAreaAtPosition( *pUnit->GetSteer()->GetPosition(), -1 ) != -1 )
			pUnit->ChangeSteer( m_pSteerClass );
	}

	return RESULT_DONE;
}
