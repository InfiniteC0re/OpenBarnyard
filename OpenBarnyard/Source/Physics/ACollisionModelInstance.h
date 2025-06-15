#pragma once

#include <Toshi/TSceneObject.h>
#include <Render/TTransformObject.h>

class ACollisionModelSet;

class ACollisionModelInstance
{
public:
	ACollisionModelInstance();
	~ACollisionModelInstance();

	void Create( ACollisionModelSet* a_pModelSet, Toshi::TSceneObject* a_pSceneObject );
	void GetTransformForBone( TINT a_iBone, Toshi::TMatrix44& a_rOutMatrix );

	TFLOAT GetScale() const;

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

private:
	ACollisionModelSet*      m_pCollisionModelSet;
	Toshi::TSceneObject*     m_pSceneObject;
	TINT                     m_iUnk;
	Toshi::TTransformObject* m_pTransformObject;
};
