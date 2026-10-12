#pragma once
#include "AOpcodeCache.h"
#include "ACollisionCommon.h"

#include <Render/TModel.h>
#include <Math/TOBBox.h>

class ACollisionModelInstance;

class ACollisionModel
{
public:
	ACollisionModel();
	virtual ~ACollisionModel();

	TBOOL Create( TINT a_iMeshIndex, Toshi::TModel* a_pModel );

	//-----------------------------------------------------------------------------
	// Collision Detection
	//-----------------------------------------------------------------------------

	TBOOL CollideRay(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TVector4&                   a_rcRayOrigin,
	    const Toshi::TVector4&                   a_rcRayDir,
	    TFLOAT&                                  a_rfMaxDistance,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eCGroupsMask,
	    TBOOL                                    a_bCulling
	);

	// Collects the hit triangles in a_pHitList (if not TNULL). a_eIgnoreGroups are the collision groups to skip.
	TBOOL CollideRay(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TVector4&                   a_rcRayOrigin,
	    const Toshi::TVector4&                   a_rcRayDir,
	    TFLOAT&                                  a_rfMaxDistance,
	    ACollisionModelRayHitList*               a_pHitList,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups,
	    TBOOL                                    a_bCulling
	);

	// Pushes a sphere (xyz is the center, w is the radius) out of the triangles. Outputs the distance to the closest triangle and its normal.
	TBOOL CollideSphere(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TVector4&                   a_rcSphere,
	    TFLOAT&                                  a_rfDistance,
	    Toshi::TVector4&                         a_rNormal,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Collects the triangles touching a sphere (xyz is the center, w is the radius) in a_pHitList.
	TBOOL CollideSphere(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TVector4&                   a_rcSphere,
	    ACollisionModelRayHitList*               a_pHitList,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Collects the triangles touching a box in a_pHitList.
	TBOOL CollideBox(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TOBBox&                     a_rcBox,
	    ACollisionModelRayHitList*               a_pHitList,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Collects the triangles touching the sphere around a box moving from a_rcPrevBox to a_rcBox in a_pHitList.
	TBOOL CollideSweptBox(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TOBBox&                     a_rcPrevBox,
	    const Toshi::TOBBox&                     a_rcBox,
	    ACollisionModelRayHitList*               a_pHitList,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Sweeps a sphere along a_rcDirection for a_rfDistance. Outputs the distance to the hit and the hit normal.
	// a_rfStepHeight is the height of the hit triangle below the bottom of the sphere (-1 if higher than a_fMaxStepHeight).
	TBOOL CollideSweptSphere(
	    ACollisionModelInstance*                 a_pModelInstance,
	    const Toshi::TVector4&                   a_rcCenter,
	    TFLOAT                                   a_fRadius,
	    const Toshi::TVector4&                   a_rcDirection,
	    TFLOAT&                                  a_rfDistance,
	    Toshi::TVector4&                         a_rNormal,
	    TFLOAT                                   a_fMaxStepHeight,
	    TFLOAT&                                  a_rfStepHeight,
	    TFLOAT&                                  a_rfPenetration,
	    TBOOL&                                   a_rbEdgeHit,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	TBOOL CollideRayFast(
	    const Toshi::TVector4&  a_rcRayOrigin,
	    const Toshi::TVector4&  a_rcRayDir,
	    TFLOAT                  a_fMaxDistance,
	    const Toshi::TMatrix44& a_rcModelTransform,
	    TFLOAT                  a_fModelScale
	);

	// Fast test of a sphere (xyz is the center, w is the radius) against the bounding sphere of the model.
	TBOOL CollideSphereFast( const Toshi::TVector4& a_rcSphere, const Toshi::TMatrix44& a_rcModelTransform, TFLOAT a_fModelScale );

	Toshi::TModelCollisionData* GetCollisionData() const;

	static void CreateStaticCache();
	static void DestroyStaticCache();

private:
	friend AOpcodeCache;
	inline static AOpcodeCache* s_pCache = TNULL;

private:
	Toshi::TVector4       m_vecTreeCenter;
	TFLOAT                m_fAverageExtent;
	Toshi::TModel*        m_pModel;
	TINT                  m_iMeshIndex;
	Opcode::Model         m_OpcodeModel;
	Opcode::MeshInterface m_MeshInterface;
	TINT                  m_aIgnoreGroups[ 2 ];
};
