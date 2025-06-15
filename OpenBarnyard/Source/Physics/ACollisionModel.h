#pragma once
#include "AOpcodeCache.h"

#include <Render/TModel.h>

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

	TBOOL CollideRayFast(
	    const Toshi::TVector4&  a_rcRayOrigin,
	    const Toshi::TVector4&  a_rcRayDir,
	    TFLOAT                  a_fMaxDistance,
	    const Toshi::TMatrix44& a_rcModelTransform,
	    TFLOAT                  a_fModelScale
	);

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
