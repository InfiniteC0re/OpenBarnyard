#pragma once
#include "Math/TVector3.h"

TOSHI_NAMESPACE_START

namespace TTMDBase
{

struct FileHeader
{
	TUINT32 m_uiMagic;
	TUINT32 m_uiZero1;
	TUINT16 m_uiVersionMajor;
	TUINT16 m_uiVersionMinor;
	TUINT32 m_uiZero2;
};

struct SkeletonHeader
{
	TCHAR  m_szTKLName[ 16 ];
	TINT32 m_iTKeyCount;
	TINT32 m_iQKeyCount;
	TINT32 m_iSKeyCount;
	TINT32 m_iTBaseIndex;
	TINT32 m_iQBaseIndex;
	TINT32 m_iSBaseIndex;
};

struct Material
{
	TCHAR szMatName[ 104 ];
	TCHAR szTextureFile[ 192 ];
};

struct MaterialsHeader
{
	TUINT32 uiZero1;
	TUINT32 uiZero2;
	TINT32  iNumMaterials;
	TUINT32 uiSectionSize;

	Material* GetMaterial( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < iNumMaterials );
		return TREINTERPRETCAST( Material*, this + 1 ) + a_iIndex;
	}
};

struct CollisionTypeS1
{
	TUINT16 uiUnk1;
	TUINT16 uiUnk2;
};

struct CollisionGroup
{
	const TCHAR*     pszName;
	TUINT            iUnk1;
	TUINT            uiNumFaces;
	TUINT            iUnk3;
	TINT             iSomeCount;
	CollisionTypeS1* pS1;
};

struct CollisionMesh
{
	TINT32          m_iBoneID;
	TVector3*       m_pVertices;
	TUINT32         m_uiNumVertices;
	TUINT16*        m_pIndices;
	TUINT32         m_uiNumIndices;
	TUINT32         m_uiNumCollTypes;
	CollisionGroup* m_pCollGroups;
};

struct CollisionHeader
{
	TINT           m_iNumMeshes;
	CollisionMesh* m_pMeshes;
};

struct Mesh
{
};

} // namespace TTMDBase

TOSHI_NAMESPACE_END
