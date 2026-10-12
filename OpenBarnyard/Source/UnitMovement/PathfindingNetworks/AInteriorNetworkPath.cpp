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
	m_iGoalIndex = -1;
	Reset();
}

AInteriorNetworkPath::BuildPathState::~BuildPathState()
{
}

// $Barnyard: FUNCTION 00555b00
void AInteriorNetworkPath::BuildPathState::Reset()
{
	m_iPathHead         = MAX_NUM_PATH_AREAS;
	m_iNumPathAreas     = 0;
	m_iGoalIndex        = -1;
	m_vPosition         = Toshi::TVector4::VEC_ZERO;
	m_iSpecialAreaIndex = -1;
	m_uiFlags           = FLAGS_NONE;
	m_vSideStepTarget   = Toshi::TVector4::VEC_ZERO;
}

// Same as Reset but keeps the goal, the next point and FLAGS_NOT_FOUND
// $Barnyard: FUNCTION 00555ab0
void AInteriorNetworkPath::BuildPathState::ClearPath()
{
	m_iNumPathAreas     = 0;
	m_iPathHead         = MAX_NUM_PATH_AREAS;
	m_iSpecialAreaIndex = -1;
	m_uiFlags &= ~FLAGS_SIDE_STEP;
	m_vSideStepTarget = Toshi::TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 00555c20
void AInteriorNetworkPath::BuildPathState::PushPathArea( TINT16 a_iAreaIndex )
{
	TASSERT( m_iPathHead > 0 );
	m_iNumPathAreas++;
	m_aPathAreas[ --m_iPathHead ] = a_iAreaIndex;
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

// $Barnyard: FUNCTION 00555bd0
void AInteriorNetworkPath::BuildPathState::SetSideStepTarget( const Toshi::TVector4& a_rcTarget )
{
	m_vSideStepTarget = a_rcTarget;
	m_uiFlags |= FLAGS_SIDE_STEP;
}

// $Barnyard: FUNCTION 00555c40
TINT AInteriorNetworkPath::BuildPathState::GetPathAreaIndex( TINT a_iIndex ) const
{
	TASSERT( a_iIndex >= 0 && a_iIndex < m_iNumPathAreas );
	return TUINT16( m_aPathAreas[ m_iPathHead + a_iIndex ] );
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
	m_fRadius        = 1.0f;
	m_iStartArea     = -1;
	m_vStartPosition = Toshi::TVector3::VEC_ZERO;
	m_uiHeightFlags  = 0;
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

// $Barnyard: FUNCTION 00555cf0
void AInteriorNetworkPath::OnSerialize( AHackedQuickSave::Buffer* a_pBuffer )
{
	// NOTE: the radius is written as an integer
	a_pBuffer->WriteInt( *TREINTERPRETCAST( TINT*, &m_fRadius ) );
	a_pBuffer->WriteInt( m_iStartArea );
	a_pBuffer->WriteVector4( m_vStartPosition );
}

// $Barnyard: FUNCTION 00555d20
void AInteriorNetworkPath::OnDeserialize( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->ReadFloat( m_fRadius );
	a_pBuffer->ReadInt( m_iStartArea );
	a_pBuffer->ReadVector4( m_vStartPosition );
}
