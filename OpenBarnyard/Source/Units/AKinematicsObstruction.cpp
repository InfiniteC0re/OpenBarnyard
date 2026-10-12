#include "pch.h"
#include "AKinematicsObstruction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail registers it with alignment 1
TDEFINE_CLASS_NORUNTIME( AKinematicsObstruction );

// $Barnyard: FUNCTION 005731d0
AKinematicsObstruction::AKinematicsObstruction()
{
}

// $Barnyard: FUNCTION 00573230
// $Barnyard: FUNCTION 00573370
AKinematicsObstruction::~AKinematicsObstruction()
{
}

// $Barnyard: FUNCTION 005747d0
const TVector4* AKinematicsObstruction::GetPosition()
{
	return &TVector4::VEC_ZERO;
}

TVector4 AKinematicsObstruction::GetDirection()
{
	// 0061d680 (shared)
	return TVector4::VEC_ZERO;
}

TVector4 AKinematicsObstruction::GetVelocity()
{
	// 0061d680 (shared)
	return TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 005731f0
const TVector2* AKinematicsObstruction::GetHalfSize()
{
	return &TVector2::VEC_ZERO;
}

// $Barnyard: FUNCTION 00573200
TVector4& AKinematicsObstruction::PredictPosition( TINT a_iUnk, TVector4& a_rOut )
{
	a_rOut = TVector4::VEC_ZERO;
	return a_rOut;
}

// $Barnyard: FUNCTION 00573240
void AKinematicsObstruction::ToLocalXZ( const TVector2& a_rcPoint, TVector2& a_rOutLocal )
{
	TVector4        vecDirection = GetDirection();
	const TVector4* pPosition    = GetPosition();

	TFLOAT fDiffX = a_rcPoint.x - pPosition->x;
	TFLOAT fDiffZ = a_rcPoint.y - pPosition->z;

	a_rOutLocal.y = vecDirection.z * fDiffZ + fDiffX * vecDirection.x;
	a_rOutLocal.x = fDiffZ * vecDirection.x + fDiffX * -vecDirection.z;
}

// $Barnyard: FUNCTION 005732e0
void AKinematicsObstruction::GetCapsuleXZ( const TVector2& a_rcCenter, TVector2& a_rOutStart, TVector2& a_rOutEnd, TFLOAT* a_pOutRadius )
{
	const TVector2* pSize        = GetHalfSize();
	TFLOAT          fHalfLength  = pSize->x;
	TVector4        vecDirection = GetDirection();

	TFLOAT fOffsetX = vecDirection.x * fHalfLength;
	TFLOAT fOffsetZ = vecDirection.z * fHalfLength;

	a_rOutStart   = a_rcCenter;
	a_rOutStart.x = fOffsetX + a_rOutStart.x;
	a_rOutStart.y = fOffsetZ + a_rOutStart.y;

	a_rOutEnd   = a_rcCenter;
	a_rOutEnd.x = a_rOutEnd.x - fOffsetX;
	a_rOutEnd.y = a_rOutEnd.y - fOffsetZ;

	*a_pOutRadius = pSize->y;
}

// $Barnyard: FUNCTION 00573390
TBOOL AKinematicsObstruction::CollideCircle( TFLOAT a_fTime, const TVector2& a_rcCenter, TFLOAT a_fRadius, TFLOAT* a_pOutPenetration )
{
	// NOTE: the time is a float, see ASteerKinematics::PredictPosition
	TINT  iTime = *TREINTERPRETCAST( TINT*, &a_fTime );
	SHAPE iType = GetShape();

	if ( iType == SHAPE_CIRCLE )
	{
		// Circle
		TVector4 vecPosition;
		PredictPosition( iTime, vecPosition );

		TFLOAT fDiffX      = a_rcCenter.x - vecPosition.x;
		TFLOAT fDiffZ      = a_rcCenter.y - vecPosition.z;
		TFLOAT fDistanceSq = fDiffX * fDiffX + fDiffZ * fDiffZ;
		TFLOAT fRadius     = GetRadius() + a_fRadius;

		if ( fRadius * fRadius < fDistanceSq )
			return TFALSE;

		if ( a_pOutPenetration )
			*a_pOutPenetration = fRadius - sqrtf( fDistanceSq );

		return TTRUE;
	}
	else if ( iType == SHAPE_BOX )
	{
		// Box
		TVector2 vecLocal;
		ToLocalXZ( a_rcCenter, vecLocal );

		const TVector2* pHalfSize = GetHalfSize();
		TFLOAT          fHalfX    = a_fRadius + pHalfSize->x;
		TFLOAT          fHalfZ    = a_fRadius + pHalfSize->y;

		if ( -fHalfX > vecLocal.x || vecLocal.x > fHalfX || -fHalfZ > vecLocal.y || vecLocal.y > fHalfZ )
			return TFALSE;

		if ( a_pOutPenetration )
			*a_pOutPenetration = 0.0f;

		return TTRUE;
	}
	else if ( iType == SHAPE_CAPSULE )
	{
		// Capsule
		TVector4 vecPosition;
		PredictPosition( iTime, vecPosition );

		TVector2 vecStart;
		TVector2 vecEnd;
		TFLOAT   fCapsuleRadius;
		GetCapsuleXZ( TVector2( vecPosition.x, vecPosition.z ), vecStart, vecEnd, &fCapsuleRadius );

		return TMath::CircleVsCapsule2D( vecStart, vecEnd, fCapsuleRadius, a_rcCenter, a_fRadius, a_pOutPenetration );
	}

	return TFALSE;
}
