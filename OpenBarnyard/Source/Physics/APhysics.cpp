#include "pch.h"
#include "APhysics.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

namespace APhysics
{

Toshi::TVector4 g_vecGravityDirection = TVector4::VEC_POSY;

// $Barnyard: FUNCTION 00657c00
void ASweptSphere::Transform( const TMatrix44& a_rcMatrix )
{
	TVector4 vEnd( vOrigin.x + vDirection.x, vOrigin.y + vDirection.y, vOrigin.z + vDirection.z, vOrigin.w );
	TFLOAT   fOriginW = vOrigin.w;

	TVector3 vTransformed;
	TMatrix44::TransformVector( vTransformed, a_rcMatrix, vOrigin.AsVector3() );
	vOrigin.Set( vTransformed.x, vTransformed.y, vTransformed.z, fOriginW );
	vOrigin.w = 1.0f;

	TMatrix44::TransformVector( vTransformed, a_rcMatrix, vEnd.AsVector3() );
	vDirection.Set( vTransformed.x - vOrigin.x, vTransformed.y - vOrigin.y, vTransformed.z - vOrigin.z, fOriginW );
	vDirection.Normalise();
}

// $Barnyard: FUNCTION 00657cf0
TINT CalculateNumSteps( TFLOAT a_fDistance, TFLOAT a_fStep )
{
	return TINT( a_fDistance / a_fStep ) + 1;
}

// Linear interpolation of xyz that doesn't overshoot a_rcEnd
// $Barnyard: FUNCTION 006b6700
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

// Sets a_rOut to the transform at a_fProgress between the two rotations/positions
static void InterpolateTransform( TMatrix44& a_rOut, const TQuaternion& a_rcStartRotation, const TQuaternion& a_rcEndRotation, const TVector4& a_rcStartPosition, const TVector4& a_rcEndPosition, TFLOAT a_fProgress )
{
	TQuaternion qRotation;
	TQuaternion::Slerp( qRotation, a_rcStartRotation, a_rcEndRotation, a_fProgress );
	qRotation.AsVector4().Divide4( sqrtf( qRotation.w * qRotation.w + qRotation.z * qRotation.z + qRotation.y * qRotation.y + qRotation.x * qRotation.x ) );

	TVector4 vPosition = LerpClamped3( a_rcStartPosition, a_rcEndPosition, a_fProgress );

	a_rOut.SetFromQuaternion( qRotation );
	a_rOut.GetTranslation() = vPosition;
}

// Copies the ray except fLength
static void CopyRay( ASweptSphere& a_rOut, const ASweptSphere& a_rcRay )
{
	a_rOut.vOrigin    = a_rcRay.vOrigin;
	a_rOut.vDirection = a_rcRay.vDirection;
	a_rOut.fRadius    = a_rcRay.fRadius;
}

// $Barnyard: FUNCTION 00657d10
TBOOL SweepTransform( const ASweptSphere& a_rcRay, ASweptSphere& a_rTempRay, ACollisionInterface<ASweptSphere>* a_pInterface, const TMatrix44& a_rcStart, const TMatrix44& a_rcEnd, TMatrix44& a_rOut, void* a_pUserData, TINT a_iNumSteps, TINT a_iNumRefinements )
{
	TQuaternion qStart;
	TMatrix44::MatToQuat( qStart, a_rcStart );

	TQuaternion qEnd;
	TMatrix44::MatToQuat( qEnd, a_rcEnd );

	TVector4 vStart = a_rcStart.GetTranslation();
	TVector4 vEnd   = a_rcEnd.GetTranslation();

	if ( qStart.AsVector4().IsEqual( qEnd.AsVector4() ) && vStart.IsEqual( vEnd ) )
	{
		a_rOut = a_rcStart;
		return TFALSE;
	}

	TFLOAT fStep     = 1.0f / a_iNumSteps;
	TFLOAT fProgress = fStep;

	if ( fStep > 1.0001f )
	{
		a_rOut = a_rcEnd;
		return TFALSE;
	}

	// Find the first step that hits something
	while ( TTRUE )
	{
		InterpolateTransform( a_rOut, qStart, qEnd, vStart, vEnd, fProgress );

		// NOTE: fLength isn't copied
		CopyRay( a_rTempRay, a_rcRay );
		a_rTempRay.Transform( a_rOut );

		if ( a_pInterface->Collide( &a_rTempRay ) )
			break;

		fProgress += fStep;

		if ( fProgress > 1.0001f )
		{
			a_rOut = a_rcEnd;
			return TFALSE;
		}
	}

	if ( fProgress > 1.0f )
		fProgress = 1.0f;

	TFLOAT fNoHit = fProgress - fStep;

	// Refine the time of the hit
	for ( TINT i = a_iNumRefinements; i != 0; i-- )
	{
		TFLOAT fMiddle = ( fProgress - fNoHit ) * 0.5f + fNoHit;

		InterpolateTransform( a_rOut, qStart, qEnd, vStart, vEnd, fMiddle );

		CopyRay( a_rTempRay, a_rcRay );
		a_rTempRay.Transform( a_rOut );

		// NOTE: the original passes the untransformed ray here
		if ( a_pInterface->CollideWithNormal( const_cast<ASweptSphere*>( &a_rcRay ), TSTATICCAST( TVector4, a_pUserData ) ) )
			fProgress = fMiddle;
		else
			fNoHit = fMiddle;
	}

	InterpolateTransform( a_rOut, qStart, qEnd, vStart, vEnd, fNoHit );
	return TTRUE;
}

// $Barnyard: FUNCTION 006581f0
TBOOL SweepTransform( const ASweptSphere& a_rcRay, ACollisionInterface<ASweptSphere>* a_pInterface, const TMatrix44& a_rcStart, const TMatrix44& a_rcEnd, TMatrix44& a_rOut, void* a_pUserData, TINT a_iNumSteps, TINT a_iNumRefinements )
{
	ASweptSphere oTempRay;
	return SweepTransform( a_rcRay, oTempRay, a_pInterface, a_rcStart, a_rcEnd, a_rOut, a_pUserData, a_iNumSteps, a_iNumRefinements );
}

} // namespace APhysics
