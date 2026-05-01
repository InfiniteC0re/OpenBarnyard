#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkPath.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00555c00
AInteriorNetworkPath::BuildPathState::BuildPathState()
{
	m_iUnk1           = -1;
	m_aPathAreas[ 0 ] = -1;
	Reset();
}

AInteriorNetworkPath::BuildPathState::~BuildPathState()
{
}

// $Barnyard: FUNCTION 00555b00
void AInteriorNetworkPath::BuildPathState::Reset()
{
	m_iPathHead       = MAX_NUM_PATH_AREAS;
	m_iNumPathAreas   = 0;
	m_iUnk1           = -1;
	m_aPathAreas[ 0 ] = -1;
	m_iUnk2           = -1;
	m_uiFlags         = 0;

	m_vPosition = Toshi::TVector4::VEC_ZERO;
	m_vUnk3     = Toshi::TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 00555c20
void AInteriorNetworkPath::BuildPathState::PushPathArea( TINT16 a_iAreaIndex )
{
	TASSERT( m_iPathHead >= 0 );
	m_iNumPathAreas++;
	m_aPathAreas[ m_iPathHead-- ] = a_iAreaIndex;
}

// $Barnyard: FUNCTION 00555b90
TBOOL AInteriorNetworkPath::BuildPathState::ReachedDestination() const
{
	return TVector4::DistanceSqXZ( m_vPosition, m_vDestination ) < 0.0001f;
}

void AInteriorNetworkPath::BuildPathState::SetState( const TVector4& a_rcTarget, const TVector4& a_rcDestination, const TVector4& a_rcForward )
{
	m_vPosition    = a_rcTarget;
	m_vDestination = a_rcDestination;
	m_vForward     = a_rcForward;
}

// $Barnyard: FUNCTION 00555c40
TINT AInteriorNetworkPath::BuildPathState::GetPathAreaIndex( TINT a_iIndex ) const
{
	TASSERT( a_iIndex >= 0 && a_iIndex < m_iNumPathAreas );
	return m_aPathAreas[ m_iPathHead + a_iIndex + 1 ];
}

// $Barnyard: FUNCTION 00555b80
void AInteriorNetworkPath::BuildPathState::AdvancePath( TINT a_iNumAreas )
{
	if ( a_iNumAreas > 0 )
	{
		m_iPathHead += a_iNumAreas;
		m_iNumPathAreas -= a_iNumAreas;
	}
}

// $Barnyard: FUNCTION 00555c50
AInteriorNetworkPath::AInteriorNetworkPath()
{
	m_fUnk1          = 1.0f;
	m_iStartArea     = -1;
	m_vStartPosition = Toshi::TVector3::VEC_ZERO;
	m_Unk3           = 0;
}

// $Barnyard: FUNCTION 00555ca0
AInteriorNetworkPath::~AInteriorNetworkPath()
{
}

// $Barnyard: FUNCTION 00555cb0
void AInteriorNetworkPath::SetStart( const Toshi::TVector4& a_rcPosition, TINT a_iAreaIndex )
{
	m_iStartArea     = a_iAreaIndex;
	m_vStartPosition = a_rcPosition;
	m_oPath.Reset();
}

// $Barnyard: FUNCTION 005516a0
TBOOL AInteriorNetworkPath::SetPathToGoal( AInteriorGoal* a_pGoal, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutForward )
{
	a_pGoal->GetPosition( a_rOutPosition );

	a_rOutForward   = a_pGoal->m_vForward;
	a_rOutForward.y = 0.0;
	a_rOutForward.Normalise();

	SetStart( a_rOutPosition, a_pGoal->m_iAreaIndex );
	m_oPath.Reset();
	m_oPath.PushPathArea( a_pGoal->m_iAreaIndex );
	m_oPath.SetState( a_rOutPosition, a_rOutPosition, a_rOutForward );

	return TTRUE;
}
