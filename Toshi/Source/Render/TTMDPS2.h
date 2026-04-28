#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "TModelCollision.h"
#include "Math/TSphere.h"

TOSHI_NAMESPACE_START

namespace TTMDPS2
{

struct TRBLODHeader
{
	TINT    m_iMeshCount1;
	TINT    m_iMeshCount2;

	// TTMDBase::SHADERTYPE, but it is only 2 bytes on PS2!
	TUINT16 m_eShader;

	TSphere m_RenderVolume;
};

struct TRBPS2Header
{
	TINT          m_iNumLODs;
	TFLOAT        m_fLODDistance;
	TRBLODHeader* m_pLODs;

	TRBLODHeader* GetLOD( TINT a_iIndex ) { return m_pLODs + a_iIndex; }
};

// SubMesh flags are set dynamically by the Engine:
// 0x14000163 for Skin
// 0x140001df for World
// 0x1400005e for StaticInstance

namespace StaticInstance
{

static constexpr TFLOAT POSITION_SCALE     = 1.0f / 128.0f;
static constexpr TFLOAT UV_SCALE           = 1.0f / 4096.0f;
static constexpr TINT   POSITIONS_HDR_SIZE = 48; // TODO: VIF packet header bytes before position data
static constexpr TINT   UV_HDR_SIZE        = 12; // TODO: header bytes before UV data
static constexpr TINT   INDEX_HDR_SIZE     = 4;  // TODO: header bytes before index data

struct SubMesh
{
	Toshi::TSphere m_BoundingSphere;
	TINT16*        m_pPositions;
	TUINT8*        m_pBoneIndices;
	TINT16*        m_pUVs;
	TUINT8*        m_pIndices;
	void*          m_pUnk; // color?
	TUINT32        m_uiNumVertices;
	TUINT32        m_uiNumIndices;
	TUINT32        m_uiFlags;
};

struct TRBMeshLODHeader
{
	SubMesh* m_pSubMeshes;
	TUINT32  m_uiNumSubMeshes;
	TUINT32  m_uiTotalVerts;
	TFLOAT   m_fUnk;
	TCHAR*   m_szMaterialName;
};

} // namespace StaticInstance

} // namespace TTMDPS2

TOSHI_NAMESPACE_END
