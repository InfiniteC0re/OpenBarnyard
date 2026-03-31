#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "TModelCollision.h"
#include "Math/TSphere.h"

TOSHI_NAMESPACE_START

namespace TTMDWin
{

struct Vertex
{
	TVector3 Position;
	TVector3 Normal;
	TUINT8   Weights[ 4 ];
	TUINT8   Bones[ 4 ];
	TVector2 UV;
};

struct SubMesh
{
	TUINT32  m_uiNumVertices1;
	TUINT16  m_uiNumVertices2;
	TUINT32  m_uiNumIndices;
	TUINT32  m_uiNumBones;
	TUINT32* m_pBones;
	Vertex*  m_pVertices;
	TUINT16* m_pIndices;
	TUINT32  m_Zero;
	TINT32   m_Unk2;
	TINT32   m_Unk3;
	TINT32   m_Unk4;
	TINT32   m_Unk5;
	TINT32   m_Unk6;
};

struct TRBMeshLODHeader
{
	TUINT    m_uiNumSubMeshes;
	TUINT32  m_uiNumIndices;
	TUINT32  m_uiNumVertices;
	TCHAR*   m_szMaterialName;
	SubMesh* m_pSubMeshes;
};

struct TRBLODHeader
{
	TINT                 m_iMeshCount1;
	TINT                 m_iMeshCount2;
	TTMDBase::SHADERTYPE m_eShader;
	TSphere              m_RenderVolume;
};

struct TRBWinHeader
{
	TINT   m_iNumLODs;
	TFLOAT m_fLODDistance;

	TRBLODHeader* GetLOD( TINT a_uiIndex )
	{
		TASSERT( a_uiIndex < m_iNumLODs );
		return TREINTERPRETCAST( TRBLODHeader*, this + 1 ) + a_uiIndex;
	}
};

}; // namespace TTMDWin

TOSHI_NAMESPACE_END
