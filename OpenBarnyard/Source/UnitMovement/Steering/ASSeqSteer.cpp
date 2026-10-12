#include "pch.h"
#include "ASSeqSteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0056fec0, CreateObject 00570160, CreateObjectInPlace 005701c0
TDEFINE_CLASS( ASSeqSteer );

// $Barnyard: FUNCTION 0056fed0
ASSeqSteer::ASSeqSteer()
    : m_vPosition( TVector4::VEC_ZERO )
    , m_vTarget( TVector4::VEC_ZERO )
    , m_vDirection( TVector4::VEC_POSX )
    , m_vDesiredDirection( TVector4::VEC_POSX )
    , m_vUnk60( TVector4::VEC_ZERO )
    , m_fTurnSpeed( 0.0f )
    , m_fSpeed( 0.0f )
    , m_bSeeking( TFALSE )
{
	// NOTE: m_bHasDesiredFacing and m_fSeekOffset are left uninitialised in retail
}

// $Barnyard: FUNCTION 0056ffc0
// $Barnyard: FUNCTION 00570200
ASSeqSteer::~ASSeqSteer()
{
}

// $Barnyard: FUNCTION 005700e0
TBOOL ASSeqSteer::HasArrived()
{
	TVector4 vecToTarget;
	vecToTarget.x = m_vTarget.x - m_vPosition.x;
	vecToTarget.y = m_vTarget.y - m_vPosition.y;
	vecToTarget.z = m_vTarget.z - m_vPosition.z;
	vecToTarget.w = m_vTarget.w;

	// NOTE: retail compares the squared distance with the (not squared) seek offset
	return vecToTarget.MagnitudeSq() <= m_fSeekOffset;
}

// $Barnyard: FUNCTION 0056ffd0
TBOOL ASSeqSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;
	m_fSpeed    = 0.0f;
	return TTRUE;
}

// $Barnyard: FUNCTION 00570000
void ASSeqSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vDesiredDirection = a_rcDirection;
	m_vDirection        = a_rcDirection;
}

// $Barnyard: FUNCTION 00570040
// $Barnyard: FUNCTION 00570057
void ASSeqSteer::CopyDirection( TVector4& a_rOutDirection )
{
	// NOTE: retail splits the copy of w into a separate chunk (00570057)
	a_rOutDirection = m_vDirection;
}

// $Barnyard: FUNCTION 0056ffb0
TFLOAT ASSeqSteer::GetSpeed()
{
	return m_fSpeed;
}

// NOTE: the same function as GetSpeed (0056ffb0)
TFLOAT ASSeqSteer::GetMeasuredSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 00570150
void ASSeqSteer::SetSpeed( TFLOAT a_fSpeed )
{
	m_fSpeed = a_fSpeed;
}

// $Barnyard: FUNCTION 00570060
TBOOL ASSeqSteer::Seek( const TVector4& a_rcTarget, TFLOAT a_fSeekOffset )
{
	m_bSeeking    = TTRUE;
	m_vTarget     = a_rcTarget;
	m_fSeekOffset = a_fSeekOffset;
	return TTRUE;
}

// $Barnyard: FUNCTION 00570090
TBOOL ASSeqSteer::HaltFacingDesired( const TVector3& a_rcFacing )
{
	m_bHasDesiredFacing = TTRUE;
	m_vDesiredDirection.Set( a_rcFacing.x, a_rcFacing.y, a_rcFacing.z, 1.0f );
	return TTRUE;
}
