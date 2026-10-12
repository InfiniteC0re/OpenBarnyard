#pragma once
#include "ACollisionObject.h"

#include <Render/TTransformObject.h>

//-----------------------------------------------------------------------------
// Purpose: Collision of a static object (ARegrowthManager, AInstanceManager):
// a capsule (a sphere if m_fLength is 0) from m_vPosition along m_vDirection.
// If the object has a collision model instance, the capsule is only the bounds
// and the collision meshes of the model (placed with m_oTransform) are tested.
// NOTE: TClass 0x007b4844, vtable 0x00737780, size 0xa4. The parent is ACollisionObject,
// some of the slots are shared with ACollisionObjectSimple (same layout of the sphere).
//-----------------------------------------------------------------------------
class ACollisionObjectModel
    : public ACollisionObject
{
public:
	TDECLARE_CLASS( ACollisionObjectModel, ACollisionObject );

public:
	ACollisionObjectModel();
	~ACollisionObjectModel();

	// Sets the capsule up and activates the object. a_pModelInstance and a_pTransform are optional (TNULL: the capsule is the collision).
	// NOTE: a_pTransform is copied, TNULL resets m_oTransform to the identity.
	TBOOL Create( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fLength, const Toshi::TVector4& a_rcDirection, TUINT a_eTypeFlags, ACollisionModelInstance* a_pModelInstance, const Toshi::TTransformObject* a_pTransform );

	//-----------------------------------------------------------------------------
	// AHashedObjectAuto
	//-----------------------------------------------------------------------------
	virtual TFLOAT                 GetRadius() OVERRIDE;   // +0x04
	virtual const Toshi::TVector4& GetPosition() OVERRIDE; // +0x08

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual void                   Update( void* ) OVERRIDE {}                                                                                                                                                                                                                 // +0x1c, DOES_NOTHING 00628f40
	virtual TBOOL                  CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit ) OVERRIDE;                                                                                                                                                                      // +0x20
	virtual TBOOL                  CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE;   // +0x24
	virtual TBOOL                  CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE;                       // +0x28
	virtual TBOOL                  CollideSphere( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal ) OVERRIDE;                                                                                               // +0x2c
	virtual void                   UpdateTransform() OVERRIDE;                                                                                                                                                                                                                 // +0x34, puts m_oTransform on the scene object of the model instance
	virtual const Toshi::TSphere&  GetObjectSphere() OVERRIDE;                                                                                                                                                                                                                 // +0x38
	virtual const Toshi::TVector4& GetObjectPosition() OVERRIDE;                                                                                                                                                                                                               // +0x3c
	virtual TFLOAT                 GetObjectRadius() OVERRIDE;                                                                                                                                                                                                                 // +0x40
	virtual void                   GetVelocity( Toshi::TVector4& a_rOutVector ) OVERRIDE;                                                                                                                                                                                      // +0x44
	virtual TFLOAT                 GetHeightScale() OVERRIDE;                                                                                                                                                                                                                  // +0x48
	virtual TBOOL                  UpdateHash( TFLOAT a_fRadius ) OVERRIDE;                                                                                                                                                                                                    // +0x4c
	virtual TBOOL                  ReciprocateSweptSphere( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE; // +0x54
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) OVERRIDE;                                                                                                                                                              // +0x58
	virtual void                   SetVelocity( void* ) OVERRIDE {}                                                                                                                                                                                                            // +0x5c, DOES_NOTHING 00628f40

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Checks if the position or the radius has changed and updates hash if needed
	virtual TBOOL UpdateHash( TFLOAT a_fRadius, const Toshi::TVector3& a_rcPosition ); // +0x60

private:
	// Tests of the capsule. NOTE: retail passes the capsule (&m_vPosition), the object and m_pModelInstance explicitly.
	TBOOL CollideRayImpl( const ARayData& a_rcRay, ACollisionHit& a_rHit );
	TBOOL CollideSweptSphereImpl( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight );
	TBOOL CollideSphereImpl( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal );
	TBOOL ReciprocateSweptSphereImpl( TFLOAT a_fRadius, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight );

	// Makes the model instance use m_oTransform (inlined at the start of every test)
	void UseTransform();

private:
	// NOTE: m_vPosition and m_fRadius form the object's TSphere (GetObjectSphere)
	Toshi::TVector3         m_vPosition;  // 0x20
	TFLOAT                  m_fRadius;    // 0x2c
	Toshi::TVector4         m_vDirection; // 0x30, direction of the capsule
	TFLOAT                  m_fLength;    // 0x40, length of the capsule
	Toshi::TTransformObject m_oTransform; // 0x44, transform of the model instance
};

TSTATICASSERT( sizeof( ACollisionObjectModel ) == 0xa4 );
