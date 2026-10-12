#pragma once
#include <Math/TMatrix44.h>

namespace APhysics
{

extern Toshi::TVector4 g_vecGravityDirection;

class ARay
{
public:
	// Distance passed to ACollisionInterface::Collide
	TFLOAT GetCollideDistance() const { return fLength; }

public:
	Toshi::TVector4 vOrigin;    // 0x00
	Toshi::TVector4 vDirection; // 0x10
	TFLOAT          fLength;    // 0x20
};

class ASphere
{
public:
	// Distance passed to ACollisionInterface::Collide
	TFLOAT GetCollideDistance() const { return fRadius; }

public:
	Toshi::TVector4 vOrigin; // 0x00
	TFLOAT          fRadius; // 0x10
};

class ASweptSphere
{
public:
	// Moves the sphere into the space of a_rcMatrix (the direction stays normalised)
	void Transform( const Toshi::TMatrix44& a_rcMatrix );

	// Distance passed to ACollisionInterface::Collide
	TFLOAT GetCollideDistance() const { return fLength; }

public:
	Toshi::TVector4 vOrigin;    // 0x00
	Toshi::TVector4 vDirection; // 0x10
	TFLOAT          fLength;    // 0x20
	TFLOAT          fRadius;    // 0x24
};

class ACollisionInterfaceCommon
{
};

//-----------------------------------------------------------------------------
// Purpose: Collides a shape (ARay, ASphere or ASweptSphere) with the world.
// NOTE: Collide and CollideWithNormal are folded for all the shapes in the
// original binary (except CollideWithNormal of ASphere, 004014b0).
//-----------------------------------------------------------------------------
template <class T>
class ACollisionInterface
    : public ACollisionInterfaceCommon
{
public:
	// $Barnyard: FUNCTION 004014d0
	virtual TBOOL Collide( T* a_pShape )
	{
		Toshi::TVector4 vNormal;
		return CollideWithNormal( a_pShape, &vNormal );
	}

	// $Barnyard: FUNCTION 00401490
	// $Barnyard: FUNCTION 004014b0
	virtual TBOOL CollideWithNormal( T* a_pShape, Toshi::TVector4* a_pNormal )
	{
		TFLOAT fDistance = a_pShape->GetCollideDistance();
		return CollideImpl( a_pShape, &fDistance, a_pNormal );
	}

	// a_pDistance is the maximum distance on input and the distance of the hit on output
	virtual TBOOL CollideImpl( T* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal ) = 0;
};

// Number of steps of the length a_fStep needed to go through a_fDistance
TINT CalculateNumSteps( TFLOAT a_fDistance, TFLOAT a_fStep );

// Moves a_rcRay from a_rcStart to a_rcEnd in a_iNumSteps steps (the rotation is slerped) until
// a_pInterface reports a hit (Collide) and refines the time of the hit a_iNumRefinements times (CollideWithNormal).
// a_rOut receives the last transform that doesn't hit anything. Returns TTRUE if something was hit.
TBOOL SweepTransform( const ASweptSphere& a_rcRay, ASweptSphere& a_rTempRay, ACollisionInterface<ASweptSphere>* a_pInterface, const Toshi::TMatrix44& a_rcStart, const Toshi::TMatrix44& a_rcEnd, Toshi::TMatrix44& a_rOut, void* a_pUserData, TINT a_iNumSteps, TINT a_iNumRefinements );
TBOOL SweepTransform( const ASweptSphere& a_rcRay, ACollisionInterface<ASweptSphere>* a_pInterface, const Toshi::TMatrix44& a_rcStart, const Toshi::TMatrix44& a_rcEnd, Toshi::TMatrix44& a_rOut, void* a_pUserData, TINT a_iNumSteps, TINT a_iNumRefinements );

} // namespace APhysics
