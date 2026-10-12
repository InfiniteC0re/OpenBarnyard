#pragma once
#include "Physics/APhysicsVehicle.h"
#include "Physics/ACollisionCommon.h"

#include <Toshi/T2Vector.h>
#include <Render/TCollisionCommon.h>

class APhysicsBike;
class APhysicsCar;
class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: Physics world of the rigid bodies and the vehicles: the lists of the
// simulated bodies and vehicles, the pools of the vehicles and the collision
// step. All data is static (retail globals at 0x007bdf88).
//-----------------------------------------------------------------------------
class AVehicleManager
{
public:
	// The surface a contact or a wheel is on (m_iSurfaceType), from the collision group of
	// the ground (GetSurfaceType), same as AAnimationBreakpointSoundParticle::SURFACE_
	using SURFACE = TINT;
	enum SURFACE_ : SURFACE
	{
		SURFACE_NONE   = -1,
		SURFACE_GRASS  = 0, // TOSHICGROUP_GRASS, also the bushes
		SURFACE_DIRT   = 1, // TOSHICGROUP_DIRT
		SURFACE_TIMBER = 2, // TOSHICGROUP_TIMBER, also the trunks and the foliage of the trees
		SURFACE_WATER  = 3, // TOSHICGROUP_WATER
		SURFACE_STONE  = 4, // TOSHICGROUP_STONE
		SURFACE_TIN    = 5, // TOSHICGROUP_TIN
		SURFACE_SAND   = 6, // TOSHICGROUP_SAND
		SURFACE_MUD    = 7, // TOSHICGROUP_MUD
		SURFACE_HAY    = 8, // TOSHICGROUP_HAY
	};

	static constexpr TINT MAX_NUM_BODIES    = 8;
	static constexpr TINT MAX_NUM_VEHICLES  = 8;
	static constexpr TINT MAX_NUM_CONTACTS  = 32;
	static constexpr TINT MAX_NUM_TRIANGLES = 512;

	// Summary of the contacts of a body after ResolveContacts, size 0x14
	struct ContactSummary
	{
		TINT            iNumContacts; // 0x00
		Toshi::TVector4 vImpulse;     // 0x04, average impulse
	};

public:
	// Steps the physics (called by ACollisionManager::UpdateVehicles at a fixed rate)
	static void UpdatePhysics( TFLOAT a_fDeltaTime );

	// Rigid bodies simulated by UpdatePhysics
	static void AddRigidBody( ARigidBody* a_pBody );
	static void RemoveRigidBody( ARigidBody* a_pBody );

	// Vehicles updated by UpdatePhysics
	static void AddVehicle( APhysicsVehicle* a_pVehicle );
	static void RemoveVehicle( APhysicsVehicle* a_pVehicle );

	// Pools of the vehicles
	static APhysicsCar*  CreateCar();
	static void          DestroyCar( APhysicsCar* a_pCar );
	static APhysicsBike* CreateBike();
	static void          DestroyBike( APhysicsBike* a_pBike );

	// Number of collision iterations of a body per step
	static void SetNumIterations( TINT a_iNumIterations );
	static TINT GetNumIterations();

	static void SetSlideOnObjects( TBOOL a_bSlide );
	static void SetUseTriangleNormal( TBOOL a_bUse );

	// Casts a ray from a_rcStart to a_rcEnd against the terrain and the collision objects.
	// Used by the wheels, the cameras and AFallingCow.
	static TBOOL CollideRay( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcEnd, ARigidBodyCollision* a_pOutCollision, TUINT a_eIgnoreTypes, Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups );

	// Collision of a body against the world (its shape moved from the previous to the current transform)
	static TBOOL CollideSphere( ARigidBody* a_pBody, const Toshi::TVector4& a_rcSphere, TFLOAT a_fDeltaTime );
	static TBOOL CollidePoints( ARigidBody* a_pBody, const Toshi::TVector4* a_pPoints, TINT a_iNumPoints, TFLOAT a_fDeltaTime );
	static TBOOL CollideBox( ARigidBody* a_pBody, const Toshi::TOBBox& a_rcBox, TFLOAT a_fDeltaTime, TBOOL a_bSwept );
	static TBOOL CollideShape( ARigidBody* a_pBody, TFLOAT a_fDeltaTime );

	// Collide and resolve the contacts in one call
	static TBOOL CollideSphereAndResolve( ARigidBody* a_pBody, const Toshi::TVector4& a_rcSphere, TFLOAT a_fDeltaTime, ContactSummary* a_pSummary );
	static TBOOL CollidePointsAndResolve( ARigidBody* a_pBody, const Toshi::TVector4* a_pPoints, TINT a_iNumPoints, TFLOAT a_fDeltaTime, ContactSummary* a_pSummary );
	static TBOOL CollideBoxAndResolve( ARigidBody* a_pBody, const Toshi::TOBBox& a_rcBox, TFLOAT a_fDeltaTime, ContactSummary* a_pSummary, TBOOL a_bSwept );

	// Applies the contacts of a body
	static TBOOL ResolveContacts( ARigidBody* a_pBody, ContactSummary* a_pSummary );

	static TBOOL BoundingSpheresOverlap( ARigidBody* a_pBody1, ARigidBody* a_pBody2 );

private:
	static TBOOL   CollideSweptSphere( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcEnd, TFLOAT a_fRadius );
	static TBOOL   CollideBoxWithObject( const Toshi::TOBBox& a_rcBox, ACollisionObject* a_pObject, Toshi::TCollisionContact* a_pContact );
	static TBOOL   CollideBoxStatic( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, const Toshi::TOBBox& a_rcPrevBox, Toshi::TOBBox& a_rBox );
	static TBOOL   CollideBoxSwept( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, const Toshi::TOBBox& a_rcPrevBox, Toshi::TOBBox& a_rBox );
	static void    AddStaticContact( ARigidBody* a_pBody, ARigidBodyCollision* a_pCollision, TFLOAT a_fDeltaTime );
	static void    AddBodyContacts( ARigidBody* a_pBody1, ARigidBody* a_pBody2, ARigidBodyCollision* a_pCollision, TFLOAT a_fDeltaTime );
	static TBOOL   CollideBodies( ARigidBody* a_pBody1, ARigidBody* a_pBody2, TFLOAT a_fDeltaTime );
	static SURFACE GetSurfaceType( const ACollisionModelRayHitList::Hit& a_rcHit );

	static ARigidBodyContact* AllocateContact();

	// Steps of UpdatePhysics
	static void ClearContacts( TFLOAT a_fDeltaTime );
	static void CollideAllBodies( TFLOAT a_fDeltaTime );
	static void CollideAllShapes( TFLOAT a_fDeltaTime );
	static void ResolveAllContacts( TFLOAT a_fDeltaTime );
	static void IntegrateAll( TFLOAT a_fDeltaTime );
	static void SendContactEvents();
	static void PostUpdateVehicles( TFLOAT a_fDeltaTime );

public:
	friend class ACollisionManager;

	inline static Toshi::T2Vector<ARigidBody*, MAX_NUM_BODIES>        ms_vecBodies;
	inline static Toshi::T2Vector<APhysicsVehicle*, MAX_NUM_VEHICLES> ms_vecVehicles;

	inline static TINT               ms_iNumUsedContacts   = 0;
	inline static TINT               ms_iMaxNumContacts    = 0;
	inline static ARigidBodyContact* ms_pContacts          = TNULL;
	inline static TBOOL              ms_bUnk7cd02c         = TFALSE;
	inline static TINT               ms_iNumIterations     = 1;
	inline static TBOOL              ms_bSlideOnObjects    = TFALSE;
	inline static TBOOL              ms_bUseTriangleNormal = TFALSE;
};
