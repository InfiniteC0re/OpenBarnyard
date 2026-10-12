#pragma once
#include "Physics/AObjectHashMain.h"
#include "Physics/ACollisionObject.h"
#include "Physics/ACollisionCommon.h"
#include "Units/AUnitProxy.h"

#include <Toshi/T2DList.h>
#include <ToshiTools/T2DynamicVector.h>
#include <Render/TCollisionCommon.h>
#include <Math/TOBBox.h>

class AUnit;

class ACollisionManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ACollisionManager>
{
public:
	TDECLARE_CLASS( ACollisionManager, Toshi::TTask );

	enum HashType : TUINT
	{
		HashType_UNK1,
		HashType_UNK2,
		HashType_UNK3,
		HashType_UNK4
	};

public:
	ACollisionManager();
	~ACollisionManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnDestroy() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	AObjectHashMain* CreateObjectHashMain( HashType a_eHashType );
	void             DestroyObjectHashMain();

	void UpdateVehicles( TFLOAT a_fDeltaTime );

	//-----------------------------------------------------------------------------
	// Collision queries
	// a_eIgnoreTypes: ACollisionObject::TYPE_ bits of the objects to skip
	// a_eIgnoreGroups: collision groups of the terrain triangles to skip
	//-----------------------------------------------------------------------------

	// Casts the ray against the terrain and the collision objects. a_rfDistance receives the distance to the closest hit.
	TBOOL CollideRay( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, TFLOAT& a_rfDistance, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TUINT a_eIgnoreTypes );

	// Casts the ray against the terrain only, a_pOutDistance is only set if something was hit.
	TBOOL CollideRayWithTerrain( const ARayData& a_rcRay, TFLOAT* a_pOutDistance, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Casts the ray against the terrain only, a_rfDistance is the ray's length if nothing was hit. a_pIgnoreUnit isn't used.
	TBOOL CollideRayWithTerrain( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, TFLOAT& a_rfDistance, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Casts the ray against the collision objects (all hits are added to a_pHits) and the terrain.
	// Returns TFALSE if the terrain is closer than the closest object.
	TBOOL CollideRayObjects( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, Toshi::T2DynamicVector<ACollisionHit>* a_pHits, ACollisionHit& a_rClosestHit, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TUINT a_eIgnoreTypes );

	// Sweeps a_pObject against the other collision objects. Returns the hit object.
	ACollisionObject* CollideSweptSphereObjects( ACollisionObject* a_pObject, Toshi::TVector4& a_rNormal, TFLOAT& a_rfDistance, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcCenter, TUINT a_eIgnoreTypes, TFLOAT a_fMaxStepHeight, TFLOAT* a_pStepHeight, TINT a_iUnused );

	// Sweeps a sphere (a temporary ACollisionObjectSimpleDummy at a_rcCenter, a_rfDistance is also its height scale) against the collision objects. Returns the hit object.
	ACollisionObject* CollideSweptSphereObjects( const Toshi::TVector4& a_rcCenter, const Toshi::TVector4& a_rcDirection, TFLOAT a_fRadius, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TUINT a_eIgnoreTypes, TFLOAT a_fMaxStepHeight, TFLOAT* a_pStepHeight, TINT a_iUnused );

	// Sweeps a sphere against the collision objects and the terrain. Returns the type of the hit object, bit 0 is set if the terrain was hit.
	// a_eFlags is the a_eIgnoreTypes of the objects, bit 0 also skips the terrain.
	TUINT CollideSweptSphere( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, ACollisionObject* a_pObject, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight, TFLOAT& a_rfPenetration, TINT a_iUnused, TUINT a_eFlags, ACollisionObject** a_ppHitObject, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Sweeps a sphere and slides it along what it hits (up to 15 times), pushing it out of objects it's embedded in.
	// Walls (a normal with y above a_fMaxSlideNormalY) reflect the direction in the XZ plane instead of sliding along them.
	// Outputs the final position and the final direction (flattened to XZ), returns TFALSE only if nothing was hit and the sphere isn't touching the terrain.
	TBOOL SweepAndSlide( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, ACollisionObject* a_pObject, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, TFLOAT a_fMaxStepHeight, TFLOAT a_fSpeedScale, TFLOAT& a_rfStepHeight, Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutDirection, TUINT a_eFlags, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups, TBOOL a_bSlopeSlowdown, TBOOL* a_pUnk, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope, TFLOAT a_fMaxSlideNormalY );

	// Tests a_pObject (at a_rcCenter) against the other collision objects. Returns the last hit object.
	ACollisionObject* CollideSphereObjects( const Toshi::TVector4& a_rcCenter, ACollisionObject* a_pObject, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TUINT a_eIgnoreTypes );

	// Tests a_pObject against the other collision objects with ACollisionObject::CollideShape.
	// a_pShape is a structure with the position at 0x30 and the radius at 0x50.
	ACollisionObject* CollideShapeObjects( void* a_pShape, ACollisionObject* a_pObject, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5, Toshi::TVector4* a_pNormal, TUINT a_eIgnoreTypes );

	// Returns TTRUE if the object can be hit by the queries.
	static TBOOL IsObjectCollidable( ACollisionObject* a_pObject, TUINT a_eIgnoreTypes );

	// Returns TTRUE if a sphere touches the terrain while moving into it.
	static TBOOL IsSphereTouchingTerrain( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, TUINT a_eFlags, TINT a_iUnused, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Scales a_rfSpeed down on steep slopes (a_rcNormal.y between a_fMinSlope and a_fMaxSlope).
	static void ScaleSpeedBySlope( TFLOAT& a_rfSpeed, const Toshi::TVector4& a_rcNormal, TFLOAT a_fMinSlope, TFLOAT a_fMaxSlope );

	// Pushes a sphere out of the terrain.
	static TBOOL PushOutOfTerrain( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, Toshi::TVector4& a_rOutPosition, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Tests a_pObject (a sphere at a_rcCentre) against the collision objects and, unless a_eIgnoreTypes has bit 0, the terrain.
	// Outputs the closest distance and the normal of the hit.
	// NOTE: Retail free function (stdcall).
	// Collects the terrain triangles touching a box in a_pHitList.
	static TBOOL CollideBoxWithTerrain( const Toshi::TOBBox& a_rcBox, ACollisionModelRayHitList* a_pHitList, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	static TBOOL CollideSphereWithWorld( const Toshi::TVector4& a_rcCentre, ACollisionObject* a_pObject, TFLOAT& a_rfDistance, Toshi::TVector4& a_rNormal, TUINT a_eIgnoreTypes, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Casts a_rcRay against the collision objects of the visible units around its origin (a_fRadius) that
	// have AUnitCollisionComponentData, then against the player. The a_uiNumIgnore units of a_ppIgnore are skipped.
	// a_rHitUnit and a_rHit receive the first hit.
	// NOTE: Retail free function (stdcall).
	static TBOOL CollideRayUnits( const ARayData& a_rcRay, TFLOAT a_fRadius, Toshi::TRef<AUnitProxy>& a_rHitUnit, ACollisionHit& a_rHit, TUINT a_uiNumIgnore, AUnit* const* a_ppIgnore );

	// Same, ignoring a_pIgnoreUnit and discarding the hit.
	// NOTE: Retail free function (stdcall). Lives next to the Veggie Patch minigame code.
	// $Barnyard: FUNCTION 004c4970
	static TBOOL CollideRayUnits( AUnit* a_pIgnoreUnit, const ARayData& a_rcRay, TFLOAT a_fRadius, Toshi::TRef<AUnitProxy>& a_rHitUnit )
	{
		ACollisionHit oHit;
		return CollideRayUnits( a_rcRay, a_fRadius, a_rHitUnit, oHit, 1, &a_pIgnoreUnit );
	}

	// Finds a visible unit with AUnitCollisionComponentData whose collision object is closer than 2 * a_fRadius to a_rcCenter.
	// The a_uiNumIgnore units of a_ppIgnore are skipped. a_rHitUnit receives the unit.
	// NOTE: Retail free function (stdcall).
	static TBOOL CollideSphereUnits( const Toshi::TVector4& a_rcCenter, TFLOAT a_fRadius, Toshi::TRef<AUnitProxy>& a_rHitUnit, TUINT a_uiNumIgnore, AUnit* const* a_ppIgnore );

	// NT08: ACollisionManager::AddCollisionObject
	void AddCollisionObject( ACollisionObject* a_pObject ) { m_oCollisionObjects.PushBack( a_pObject ); }

private:
	void CreateStatic();
	void DestroyStatic();

	inline static TBOOL ms_bDebugDrawUsedBBox = TFALSE;

public:
	// Enables or disables the vehicles update in OnUpdate (ACarChaseMiniGameState)
	void SetVehiclesUpdateEnabled( TBOOL a_bEnable ) { m_bEnableVehiclesUpdate = a_bEnable; }

public:
	// Hits of the ray queries of the physics (AVehicleManager::CollideRay), 0x007b5f80
	inline static Toshi::T2DynamicVector<ACollisionHit>* ms_pRayHits = TNULL;

private:
	Toshi::T2DList<ACollisionObject> m_oCollisionObjects; // 0x20
	TINT                             field2_0x28;         // 0x28
	// ...
	TFLOAT m_fVehiclesUpdateTime;
	TFLOAT m_fVehiclesUpdateRate;
	TBOOL  m_bEnableVehiclesUpdate;
};
