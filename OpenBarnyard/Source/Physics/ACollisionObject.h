#pragma once
#include "AObjectHash.h"
#include "AHashedObjectAuto.h"
#include "ACollisionCommon.h"

#include <Toshi/T2DList.h>

class ACollisionModelInstance;

class ACollisionObject
    : public AHashedObjectAuto
    , public Toshi::TObject
    , public Toshi::T2DList<ACollisionObject>::Node
{
public:
	TDECLARE_CLASS( ACollisionObject, Toshi::TObject );

	// The class of the object (bits 1-3) and its collision flags. Also the
	// AUnitCollisionComponentData::m_iCollideFlags ("CollideFlags") of units and
	// the a_eIgnoreTypes masks of ACollisionManager.
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_NONE     = 0,
		TYPE_TERRAIN  = BITFLAG( 0 ),  // never set on objects, a_eIgnoreTypes skips the terrain and the hit results have it for the terrain
		TYPE_SIMPLE   = BITFLAG( 1 ),  // ACollisionObjectSimple, ACollisionObjectModel, ACollisionObjectBox
		TYPE_UNIT     = BITFLAG( 2 ),  // ACollisionObjectUnit
		TYPE_MESH     = BITFLAG( 3 ),  // ACollisionObjectMesh, also AUnitCollisionComponentData::m_iCollideFlags of units using it
		TYPE_TRUNK    = BITFLAG( 4 ),  // the trunks of ATreeManager's trees, the vehicles don't knock these regrowth objects over
		TYPE_FOLIAGE  = BITFLAG( 5 ),  // the foliage spheres (FOBs) of ATreeManager's trees
		TYPE_INSTANCE = BITFLAG( 6 ),  // AInstanceManager and ARegrowthManager models
		TYPE_PLAYER   = BITFLAG( 7 ),  // the player (APlayerSteer) and the player bike
		TYPE_BUSH     = BITFLAG( 8 ),  // ATreeManager's bushes, the vehicles play the bush effect
		TYPE_MOVABLE  = BITFLAG( 9 ),  // the regrowth crates, AVehicleManager treats them like spheres
		TYPE_GATE     = BITFLAG( 10 ), // AGateManager
		TYPE_IGNORED  = BITFLAG( 11 ), // skipped by the cameras and set temporarily to skip an object
	};

public:
	ACollisionObject();
	~ACollisionObject();

	AObjectHash::CellObject* CreateHash();

	// Adds the object to ACollisionManager and hashes it.
	void Activate();

	//-----------------------------------------------------------------------------
	// AHashedObjectAuto
	//-----------------------------------------------------------------------------
	virtual TFLOAT                 GetRadius() OVERRIDE;
	virtual const Toshi::TVector4& GetPosition() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Update( void* ) = 0;

	// Tests the ray against this object. a_rHit.fDistance is the max distance and receives the distance to the hit.
	virtual TBOOL CollideRay( const ARayData& a_rcRay, ACollisionHit& a_rHit ) = 0; // +0x20

	// Sweeps this object (moving from a_rcCenter along a_rcDirection for a_rfDistance) against a_pObstacle.
	// a_rfDistance receives the distance to the hit and a_rNormal the hit normal.
	virtual TBOOL CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) = 0; // +0x24

	// Same as CollideSweptSphere but the hit normal is discarded.
	virtual TBOOL CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ) = 0; // +0x28

	// Tests this object (at a_rcCenter) against a_pObstacle. a_rfDistance receives the distance and a_rNormal the normal.
	virtual TBOOL CollideSphere( ACollisionObject* a_pObstacle, const Toshi::TVector4& a_rcCenter, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal ) = 0; // +0x2c

	virtual TBOOL                  CollideShape( void*, void*, void*, void*, void*, void* );
	virtual void                   UpdateTransform();
	virtual const Toshi::TSphere&  GetObjectSphere()                            = 0; // VTable: 0x38
	virtual const Toshi::TVector4& GetObjectPosition()                          = 0; // VTable: 0x3C
	virtual TFLOAT                 GetObjectRadius()                            = 0; // VTable: 0x40
	virtual void                   GetVelocity( Toshi::TVector4& a_rOutVector ) = 0; // VTable: 0x44
	virtual TFLOAT                 GetHeightScale()                             = 0; // VTable: 0x48
	virtual TBOOL                  UpdateHash( TFLOAT a_fRadius );                   // VTable: 0x4C
	virtual void*                  GetOwnerUnit();
	virtual TBOOL                  ReciprocateSweptSphere( ACollisionObject* a_pObject, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight ); // VTable: 0x54, runs a_pObject's CollideSweptSphere against this object
	virtual void                   TranslateBy( const Toshi::TVector4& a_rDirection, TFLOAT a_fLength ) = 0;                                                                                                                                                          // VTable: 0x58
	virtual void                   SetVelocity( void* )                                                 = 0;                                                                                                                                                          // return value is unknown

	TBOOL IsCollisionObjectLinked() const { return Node::IsLinked(); }

	ACollisionModelInstance* GetModelInstance() const { return m_pModelInstance; }

	TYPE GetType() const { return m_eType; }
	void SetType( TYPE a_eType ) { m_eType = a_eType; }
	void UnlinkCollisionObject();

protected:
	ACollisionModelInstance* m_pModelInstance; // 0x18, freed (TFree) by ACollisionObjectSimple's destructor
	TYPE                     m_eType;          // 0x1c
};
