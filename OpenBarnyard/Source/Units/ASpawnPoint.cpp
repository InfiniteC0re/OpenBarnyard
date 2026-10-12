#include "pch.h"
#include "ASpawnPoint.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005270a0, CreateObject 00527170, CreateObjectInPlace 005271d0 (the constructor is inlined in both)
TDEFINE_CLASS( ASpawnPoint );

// $Barnyard: FUNCTION 005270b0
ASpawnPoint::ASpawnPoint()
{
	m_vDirection   = TVector4::VEC_ZERO;
	m_fRadius      = 2.0f;
	m_uiColour     = 0xAA00AAAA;
	m_bPositionSet = TFALSE;
	m_iUnk34       = 3;
}

ASpawnPoint::~ASpawnPoint()
{
}

// $Barnyard: FUNCTION 00527110
ASpawnPoint& ASpawnPoint::operator=( const ASpawnPoint& a_rcOther )
{
	m_vPosition  = a_rcOther.m_vPosition;
	m_uiColour   = a_rcOther.m_uiColour;
	m_iArea      = a_rcOther.m_iArea;
	m_vDirection = a_rcOther.m_vDirection;
	return *this;
}

// $Barnyard: FUNCTION 00527210
TBOOL ASpawnPoint::IsWithinDistance( const TVector4& a_rcPosition, TFLOAT a_fDistance )
{
	TVector4 vDiff( m_vPosition.x - a_rcPosition.x, m_vPosition.y - a_rcPosition.y, m_vPosition.z - a_rcPosition.z, m_vPosition.w );
	return vDiff.Magnitude() < a_fDistance;
}

// $Barnyard: FUNCTION 00527280
const TVector4* ASpawnPoint::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition    = a_rcPosition;
	m_bPositionSet = TTRUE;
	return &m_vPosition;
}
