#pragma once
#include "ACollisionObject.h"

#include <Math/TOBBox.h>
#include <Math/TSphere.h>

//-----------------------------------------------------------------------------
// Purpose: Collision object of an oriented box. Created on the stack by the box
// queries of the physics (AVehicleManager) to test the box against the other
// collision objects with CollideShape (ACollisionManager::CollideShapeObjects).
// TClass 0x007b47d0, vtable 0x00737648, size 0x88.
//-----------------------------------------------------------------------------
class ACollisionObjectBox
    : public ACollisionObject
{
public:
	TDECLARE_CLASS( ACollisionObjectBox, ACollisionObject );

	// List of the objects hit by CollideShape
	struct ObjectList
	{
		TINT              iNumObjects;
		ACollisionObject* apObjects[ 32 ];
	};

public:
	ACollisionObjectBox();
	~ACollisionObjectBox();

	void SetExtents( const Toshi::TVector4& a_rcExtents );
	void SetTransform( const Toshi::TMatrix44& a_rcTransform );

	const Toshi::TOBBox& GetBox() const { return m_oBox; }

	//-----------------------------------------------------------------------------
	// AHashedObject
	//-----------------------------------------------------------------------------
	virtual TFLOAT                 GetRadius() OVERRIDE;   // +0x04
	virtual const Toshi::TVector4& GetPosition() OVERRIDE; // +0x08

	//-----------------------------------------------------------------------------
	// ACollisionObject
	//-----------------------------------------------------------------------------
	virtual void  Update( void* ) OVERRIDE {}                                                                                                                                                                                                               // +0x1c, DOES_NOTHING 00628f40
	virtual TBOOL CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit ) OVERRIDE { return TFALSE; }                                                                                                                                                  // +0x20, RETURNS_FALSE 00556a00
	virtual TBOOL CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE; // +0x24
	virtual TBOOL CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE { return TFALSE; }   // +0x28, 0052de30
	virtual TBOOL CollideSphere( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal ) OVERRIDE;                                                                                             // +0x2c

	// Tests the box a_pShape (TOBBox) against a_pOther. a_pHitList (ACollisionModelRayHitList) receives the hit triangles of
	// collision models, a_pObjects (ObjectList) the hit object.
	virtual TBOOL CollideShape( void* a_pOther, void* a_pShape, void* a_pHitList, void* a_pObjects, void* a_pUnk5, void* a_pUnk6 ) OVERRIDE; // +0x30

	virtual void                   UpdateTransform() OVERRIDE {}                                                                                                                                                                                                                                 // +0x34, DOES_NOTHING 005acd80
	virtual const Toshi::TSphere&  GetObjectSphere() OVERRIDE;                                                                                                                                                                                                                                   // +0x38
	virtual const Toshi::TVector4& GetObjectPosition() OVERRIDE;                                                                                                                                                                                                                                 // +0x3c
	virtual TFLOAT                 GetObjectRadius() OVERRIDE;                                                                                                                                                                                                                                   // +0x40
	virtual void                   GetVelocity( Toshi::TVector4& a_rOutVector ) OVERRIDE;                                                                                                                                                                                                        // +0x44
	virtual TFLOAT                 GetHeightScale() OVERRIDE { return 0.0f; }                                                                                                                                                                                                                    // +0x48, 0056ce20
	virtual TBOOL                  UpdateHash( TFLOAT a_fRadius ) OVERRIDE { return TTRUE; }                                                                                                                                                                                                     // +0x4c, RETURNS_TRUE 00413920
	virtual void*                  GetOwnerUnit() OVERRIDE { return TNULL; }                                                                                                                                                                                                                     // +0x50, RETURNS_ZERO 0054d8b0
	virtual TBOOL                  ReciprocateSweptSphere( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) OVERRIDE { return TFALSE; } // +0x54, 0061d590
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) OVERRIDE {}                                                                                                                                                                              // +0x58, DOES_NOTHING 0054df00
	virtual void                   SetVelocity( void* ) OVERRIDE {}                                                                                                                                                                                                                              // +0x5c, DOES_NOTHING 00628f40

private:
	Toshi::TOBBox  m_oBox;    // 0x20
	Toshi::TSphere m_oSphere; // 0x78, never set
};

TSTATICASSERT( sizeof( ACollisionObjectBox ) == 0x88 );
