#pragma once
#include "APhysics.h"

#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: Collides rays, spheres and swept spheres with the collision world
// (ACollisionManager) using a temporary ACollisionObjectSimpleDummy.
// NOTE: empty_bases keeps the retail size (three vtable pointers)
//-----------------------------------------------------------------------------
class __declspec( empty_bases ) ABarnyardCollision
    : public APhysics::ACollisionInterface<APhysics::ARay>
    , public APhysics::ACollisionInterface<APhysics::ASphere>
    , public APhysics::ACollisionInterface<APhysics::ASweptSphere>
    , public Toshi::TSingleton<ABarnyardCollision>
{
public:
	ABarnyardCollision();
	virtual ~ABarnyardCollision();

	//-----------------------------------------------------------------------------
	// APhysics::ACollisionInterface
	//-----------------------------------------------------------------------------
	virtual TBOOL CollideImpl( APhysics::ARay* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal ) OVERRIDE;
	virtual TBOOL CollideImpl( APhysics::ASphere* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal ) OVERRIDE;
	virtual TBOOL CollideImpl( APhysics::ASweptSphere* a_pShape, TFLOAT* a_pDistance, Toshi::TVector4* a_pNormal ) OVERRIDE;
};

TSTATICASSERT( sizeof( ABarnyardCollision ) == 0xc );
