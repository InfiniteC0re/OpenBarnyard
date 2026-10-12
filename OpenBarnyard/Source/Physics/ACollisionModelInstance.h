#pragma once
#include "ACollisionCommon.h"

#include <Toshi/TSceneObject.h>
#include <Render/TTransformObject.h>
#include <Render/TCollisionCommon.h>
#include <Render/TModelCollision.h>
#include <Math/TOBBox.h>

class ACollisionModelSet;

class ACollisionModelInstance
{
public:
	ACollisionModelInstance();
	~ACollisionModelInstance();

	void Create( ACollisionModelSet* a_pModelSet, Toshi::TSceneObject* a_pSceneObject );
	void GetTransformForBone( TINT a_iBone, Toshi::TMatrix44& a_rOutMatrix );

	TFLOAT GetScale() const;

	// Updates the collision models of the instance (they don't do anything).
	void Update();

	// Returns collision data of the a_iModel-th collision model of the model set.
	Toshi::TModelCollisionData* GetCollisionData( TINT a_iModel ) const;

	Toshi::TSceneObject*     GetSceneObject() const { return m_pSceneObject; }
	Toshi::TTransformObject* GetTransformObject() const { return m_pTransformObject; }
	void                     SetTransformObject( Toshi::TTransformObject* a_pTransformObject ) { m_pTransformObject = a_pTransformObject; }

	//-----------------------------------------------------------------------------
	// Collision Detection
	//-----------------------------------------------------------------------------
	TBOOL CollideRay(
	    const Toshi::TVector4&                   a_rcRayOrigin,
	    const Toshi::TVector4&                   a_rcRayDir,
	    TFLOAT&                                  a_rfMaxDistance,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eCGroupsMask,
	    TBOOL                                    a_bCulling
	);

	// Collects the hit triangles in a_pHitList (if not TNULL).
	TBOOL CollideRay(
	    const Toshi::TVector4&                   a_rcRayOrigin,
	    const Toshi::TVector4&                   a_rcRayDir,
	    TFLOAT&                                  a_rfMaxDistance,
	    ACollisionModelRayHitList*               a_pHitList,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups,
	    TBOOL                                    a_bCulling
	);

	// Collects the triangles touching a sphere (xyz is the center, w is the radius) in a_pHitList.
	TBOOL CollideSphere(
	    const Toshi::TVector4&                   a_rcSphere,
	    ACollisionModelRayHitList*               a_pHitList,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Collects the triangles touching a box in a_pHitList.
	TBOOL CollideBox(
	    const Toshi::TOBBox&                     a_rcBox,
	    ACollisionModelRayHitList*               a_pHitList,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Collects the triangles touching the sphere around a box moving from a_rcPrevBox to a_rcBox in a_pHitList.
	TBOOL CollideSweptBox(
	    const Toshi::TOBBox&                     a_rcPrevBox,
	    const Toshi::TOBBox&                     a_rcBox,
	    ACollisionModelRayHitList*               a_pHitList,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Outputs the distance to the closest triangle touching a sphere (xyz is the center, w is the radius) and its normal.
	TBOOL CollideSphere(
	    const Toshi::TVector4&                   a_rcSphere,
	    TFLOAT&                                  a_rfDistance,
	    Toshi::TVector4&                         a_rNormal,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Pushes a sphere (xyz is the center, w is the radius) out of the triangles. Returns TTRUE if it was moved.
	TBOOL PushOutSphere(
	    const Toshi::TVector4&                   a_rcSphere,
	    Toshi::TVector4&                         a_rOutPosition,
	    TINT                                     a_iCollGroup,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	TBOOL CollideSweptSphere(
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

	TBOOL CollideSweptSphere(
	    const Toshi::TVector4&                   a_rcCenter,
	    TFLOAT                                   a_fRadius,
	    const Toshi::TVector4&                   a_rcDirection,
	    TFLOAT&                                  a_rfDistance,
	    Toshi::TVector4&                         a_rNormal,
	    TFLOAT                                   a_fMaxStepHeight,
	    TFLOAT&                                  a_rfStepHeight,
	    TFLOAT&                                  a_rfPenetration,
	    Toshi::TCollisionCommon::TOSHICGROUPFLAG a_eIgnoreGroups
	);

	// Same as the above but the collision groups aren't ignored and the edge hit flag is discarded.
	TBOOL CollideSweptSphere(
	    const Toshi::TVector4& a_rcCenter,
	    TFLOAT                 a_fRadius,
	    const Toshi::TVector4& a_rcDirection,
	    TFLOAT&                a_rfDistance,
	    Toshi::TVector4&       a_rNormal,
	    TFLOAT                 a_fMaxStepHeight,
	    TFLOAT&                a_rfStepHeight,
	    TFLOAT&                a_rfPenetration
	);

	// Sweeps a sphere and slides it along the hit triangles (up to 15 times). Outputs the end position.
	// Returns TFALSE if nothing was hit.
	TBOOL SlideSweptSphere(
	    const Toshi::TVector4& a_rcCenter,
	    TFLOAT                 a_fRadius,
	    const Toshi::TVector4& a_rcDirection,
	    TFLOAT&                a_rfDistance,
	    TFLOAT                 a_fMaxStepHeight,
	    TFLOAT                 a_fSpeedScale,
	    TFLOAT&                a_rfStepHeight,
	    Toshi::TVector4&       a_rOutPosition,
	    TFLOAT                 a_fMinSlope,
	    TFLOAT                 a_fMaxSlope
	);

private:
	ACollisionModelSet*      m_pCollisionModelSet; // 0x00
	Toshi::TSceneObject*     m_pSceneObject;       // 0x04
	TINT                     m_iUnk;               // 0x08
	Toshi::TTransformObject* m_pTransformObject;   // 0x0c
};
