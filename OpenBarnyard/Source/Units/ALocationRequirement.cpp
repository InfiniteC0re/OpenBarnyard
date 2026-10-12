#include "pch.h"
#include "ALocationRequirement.h"
#include "AUnit.h"
#include "GameInterface/AGameStateController.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ad030, CreateObject 005ad590
// NOTE: GetClass 005ad030, CreateObject 005ad590, CreateObjectInPlace 005ad610
TDEFINE_CLASS( ALocationRequirement );

TPSTRING8_DECLARE( barn );
TPSTRING8_DECLARE( ABarnExploreMinigameState );
TPSTRING8_DECLARE( ASimState );

// $Barnyard: FUNCTION 005ad230
// $Barnyard: FUNCTION 005ad680
ALocationRequirement::~ALocationRequirement()
{
}

// $Barnyard: FUNCTION 005ad2d0
TBOOL ALocationRequirement::Create( const PBProperties* a_pProperties )
{
	a_pProperties->GetOptionalFloatOrIntValue( m_fMinRadiusSq, TPS8D( "minradius" ) );
	a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadiusSq, TPS8D( "maxradius" ) );

	m_fMinRadiusSq = m_fMinRadiusSq * m_fMinRadiusSq;
	m_fMaxRadiusSq = m_fMaxRadiusSq * m_fMaxRadiusSq;

	if ( a_pProperties->GetOptionalPropertyValue( m_strLocation, TPS8D( "location" ) ) )
	{
		// An area of an interior network
		a_pProperties->GetOptionalPropertyValue( m_strNetwork, TPS8D( "network" ) );

		TVector4 vecOffset = TVector4::VEC_ZERO;
		a_pProperties->GetOptionalVector4( vecOffset, TPS8D( "offset" ) );
		SetPosition( vecOffset );
	}
	else if ( TVector4 vecPosition; a_pProperties->GetOptionalVector4( vecPosition, TPS8D( "position" ) ) )
	{
		SetPosition( vecPosition );
	}
	else
	{
		// The world
		a_pProperties->GetOptionalPropertyValue( m_strLocation, TPS8D( "world" ) );

		if ( m_strLocation == TPS8( barn ) )
			m_strLocation = TPS8( ABarnExploreMinigameState );
		else
			m_strLocation = TPS8( ASimState );
	}

	return AInteractionRequirement::Create( a_pProperties );
}

void ALocationRequirement::SetPosition( const TVector4& a_rcPosition )
{
	m_vecPosition.x = a_rcPosition.x;
	m_vecPosition.y = a_rcPosition.y;
	m_vecPosition.z = a_rcPosition.z;
	m_fPositionW    = a_rcPosition.w;
}

// $Barnyard: FUNCTION 005ad040
void ALocationRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	if ( m_strNetwork.IsEmpty() )
	{
		// The game state is on the stack
		TBOOL bAllowed = AGameStateController::GetSingleton()->FindGameStateOnStack( m_strLocation ) != TNULL;

		for ( TINT i = 0; i < iNumUnits; i++ )
			SetUnitAllowed( m_eCategory, i, bAllowed );
	}
	else if ( m_strLocation.IsEmpty() )
	{
		// The distance to the position
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			TVector4 vecPosition( m_vecPosition.x, m_vecPosition.y, m_vecPosition.z, m_fPositionW );
			TFLOAT   fDistanceSq = TVector4::DistanceSq( *a_pParticipants->Get( i )->GetSteer()->GetPosition(), vecPosition );
			SetUnitAllowed( m_eCategory, i, fDistanceSq >= m_fMinRadiusSq && fDistanceSq <= m_fMaxRadiusSq );
		}
	}
	else
	{
		// The (XZ) distance to the area of the network
		AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->FindNetwork( m_strNetwork );

		if ( !pNetwork )
		{
			for ( TINT i = 0; i < iNumUnits; i++ )
				SetUnitAllowed( m_eCategory, i, TFALSE );

			return;
		}

		AInteriorArea* pArea = &pNetwork->m_pINetwork->m_pAreas[ pNetwork->FindAreaIndex( m_strLocation ) ];
		TFLOAT         fX    = pArea->m_fOriginX + m_vecPosition.x;
		TFLOAT         fZ    = pArea->m_fOriginZ + m_vecPosition.z;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			const TVector4* pPosition   = a_pParticipants->Get( i )->GetSteer()->GetPosition();
			TFLOAT          fDeltaX     = fX - pPosition->x;
			TFLOAT          fDeltaZ     = fZ - pPosition->z;
			TFLOAT          fDistanceSq = fDeltaX * fDeltaX + fDeltaZ * fDeltaZ;

			SetUnitAllowed( m_eCategory, i, fDistanceSq >= m_fMinRadiusSq && fDistanceSq <= m_fMaxRadiusSq );
		}
	}
}
