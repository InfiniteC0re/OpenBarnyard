#include "pch.h"
#include "ACarChaseCamera.h"
#include "ACamera.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ACarChaseCamera );

// $Barnyard: FUNCTION 00460f60
ACarChaseCamera::ACarChaseCamera()
    : m_vUnk130( TVector4::VEC_POSZ )
    , m_vUnk140( TVector4::VEC_NEGZ )
    , m_vCarPosition( TVector4::VEC_ZERO )
    , m_iViewIndex( -1 )
    , m_iUnk188( 0 )
    , m_fOffsetSpeed( 25.0f )
    , m_fCameraSpeed( 2.2f )
    , m_bHolding( TTRUE )
    , m_vPositionOffset( TVector4::VEC_ZERO )
    , m_vTargetOffset( TVector4::VEC_POSX )
    , m_iLastViewIndex( -2 )
{
}

// $Barnyard: FUNCTION 00460f50
// $Barnyard: FUNCTION 00461080
ACarChaseCamera::~ACarChaseCamera()
{
}

// Linear interpolation of xyz that doesn't overshoot a_vEnd
// NOTE: a copy of LerpClamped3 (006b6700, static in Physics/APhysics.cpp)
static TVector4 LerpClamped3( TVector4 a_vStart, TVector4 a_vEnd, TFLOAT a_fProgress )
{
	TFLOAT afResult[ 3 ];

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fStart = a_vStart.AsArray()[ i ];
		TFLOAT fEnd   = a_vEnd.AsArray()[ i ];
		TFLOAT fValue = ( fEnd - fStart ) * a_fProgress + fStart;

		if ( ( fStart < fEnd && fValue > fEnd ) || ( fStart > fEnd && fValue < fEnd ) )
			fValue = fEnd;

		afResult[ i ] = fValue;
	}

	return TVector4( afResult[ 0 ], afResult[ 1 ], afResult[ 2 ], a_vStart.w );
}

// $Barnyard: FUNCTION 004609a0
TVector4 ACarChaseCamera::TransformOffset( TVector4 a_vOffset )
{
	// Heading of the car on the ground plane
	TVector4 vHeading;
	vHeading.x = m_vUnk130.x - m_vUnk140.x;
	vHeading.y = 0.0f;
	vHeading.z = m_vUnk130.z - m_vUnk140.z;
	vHeading.w = m_vUnk130.w;
	vHeading.Normalise();

	TVector4 vUp = TVector4::VEC_NEGY;

	TVector4 vRight;
	vRight.CrossProduct( vHeading, vUp );
	vRight.Normalise();

	// NOTE: the rows are right, heading and up, so the offset gets swizzled to (x, z, -y)
	TMatrix44 mTransform                                 = TMatrix44::IDENTITY;
	mTransform.AsBasisVector3( BASISVECTOR_RIGHT )       = vRight.AsVector3();
	mTransform.AsBasisVector3( BASISVECTOR_UP )          = vHeading.AsVector3();
	mTransform.AsBasisVector3( BASISVECTOR_FORWARD )     = vUp.AsVector3();
	mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = m_vCarPosition;

	TVector4 vLocal( a_vOffset.x, a_vOffset.z, -a_vOffset.y, a_vOffset.w );

	TVector4 vResult;
	TMatrix44::TransformVector( vResult, mTransform, vLocal );

	return vResult;
}

// $Barnyard: FUNCTION 00460b30
TBOOL ACarChaseCamera::OnUpdate( TFLOAT a_fDeltaTime )
{
	TVector4 vPosition = TransformOffset( m_avPositionOffsets[ m_iViewIndex ] );
	TVector4 vTarget   = TransformOffset( m_avTargetOffsets[ m_iViewIndex ] );

	TVector4 vPositionOffset;
	vPositionOffset.Substract( vPosition, m_vCarPosition );

	TVector4 vTargetOffset;
	vTargetOffset.Substract( vTarget, m_vCarPosition );

	// Smooth the offsets from the car
	m_vPositionOffset = LerpClamped3( m_vPositionOffset, vPositionOffset, a_fDeltaTime * m_fOffsetSpeed );
	m_vTargetOffset   = LerpClamped3( m_vTargetOffset, vTargetOffset, a_fDeltaTime * m_fOffsetSpeed );

	vPosition.Add( m_vCarPosition, m_vPositionOffset );
	vTarget.Add( m_vCarPosition, m_vTargetOffset );

	if ( m_iLastViewIndex != m_iViewIndex )
	{
		// The view has changed, cut to it
		m_vCameraTarget   = vTarget;
		m_vCameraPosition = vPosition;
		m_iLastViewIndex  = m_iViewIndex;
	}

	// Hold the camera in place while the car is close to it (with a bit of hysteresis)
	m_bHolding = TVector4::DistanceSq( m_vCarPosition, m_vCameraPosition ) < m_afHoldDistancesSq[ m_iViewIndex ] ||
	    ( m_bHolding == TTRUE && m_afHoldDistancesSq[ m_iViewIndex ] + 3.0f > TVector4::DistanceSq( m_vCarPosition, m_vCameraPosition ) );

	if ( m_bHolding == TTRUE )
	{
		// Only turn to the car
		m_vCameraTarget = LerpClamped3( m_vCameraTarget, m_vCarPosition, a_fDeltaTime * m_fCameraSpeed );
	}
	else
	{
		m_vCameraPosition = LerpClamped3( m_vCameraPosition, vPosition, a_fDeltaTime * m_fCameraSpeed );
		m_vCameraTarget   = LerpClamped3( m_vCameraTarget, vTarget, a_fDeltaTime * m_fCameraSpeed );
	}

	GetCamera()->LookAt( m_vCameraPosition, m_vCameraTarget );
	return TTRUE;
}
