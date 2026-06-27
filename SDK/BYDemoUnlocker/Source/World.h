#pragma once
#include <Render/TMesh.h>
#include <Render/TShader.h>
#include <Math/TVector2.h>
#include <Math/TVector3.h>
#include <Math/TPlane.h>
#include <Math/TSphere.h>

#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

#include <Platform/DX8/TRenderInterface_DX8.h>

struct CullBox
{
	TFLOAT x;
	TFLOAT y;
	TFLOAT width;
	TFLOAT height;
};

struct Frustum
{
	Toshi::TPlane aPlanes[ 6 ];
};

struct WorldVertex
{
	Toshi::TVector3 Position;
	Toshi::TVector3 Normal;
	Toshi::TVector3 Color;
	Toshi::TVector2 UV;
};

struct CellMesh
{
	Toshi::TMesh* pMesh;
	TUINT32       uiNumIndices;
	TUINT32       uiNumVertices1;
	TUINT16       uiNumVertices2;
	TCHAR*        szMaterialName;
	WorldVertex*  pVertices;
	TUINT16*      pIndices;
};

struct CellMeshSphere
{
	Toshi::TSphere m_BoundingSphere;
	CellMesh*      m_pCellMesh;
};

struct CellSphereTreeLeafNode
{
	TUINT32 m_uiNumMeshes;

	TUINT16 GetMeshIndex( TUINT32 a_uiIndex )
	{
		TASSERT( a_uiIndex < m_uiNumMeshes );
		return *( (TUINT16*)( this + 1 ) + a_uiIndex );
	}
};

struct CellSphereTreeBranchNode
{
	Toshi::TSphere            m_BoundingSphere;
	CellSphereTreeBranchNode* m_pRight;

	TBOOL IsLeaf() const { return m_pRight == TNULL; }

	CellSphereTreeLeafNode* GetLeafNode()
	{
		TASSERT( m_pRight == TNULL );
		return (CellSphereTreeLeafNode*)( this + 1 );
	}

	CellSphereTreeBranchNode* GetSubNode()
	{
		TASSERT( m_pRight != TNULL );
		return (CellSphereTreeBranchNode*)( this + 1 );
	}
};

struct Cell
{
	TCHAR                     m_MetaData[ 128 ];
	TINT                      m_Unknown;
	TUINT32                   uiNumMeshes;
	CellMeshSphere**          ppCellMeshSpheres;
	CellSphereTreeBranchNode* pTreeBranchNodes;
};

struct World
{
	TINT32 m_iNumCells;
	Cell** m_ppCells;
};

struct WorldDatabase
{
	TUINT32 m_uiNumWorlds;
	World** m_ppWorlds;
};

class AWorldMesh : public Toshi::TMesh
{
public:
	struct SubMesh
	{
		TMesh*                              pAssociatedMesh = 0;
		TUINT16                             uiUnk2          = 0;
		TUINT16                             uiNumVertices   = 0;
		Toshi::TIndexPoolResourceInterface* pIndexPool      = TNULL;
	};

	static constexpr TUINT NUM_SUBMESHES = 1;
	using LockBuffer                     = Toshi::TVertexPoolResourceInterface::LockBuffer;

	SubMesh* GetSubMesh( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_SUBMESHES );
		return &m_pSubMeshes[ a_uiIndex ];
	}

	TBOOL Lock( LockBuffer& a_rLockBuffer )
	{
		TASSERT( 0 == ( m_uiFlags & FLAG_LOCKED ) );

		if ( m_pVertexPool->Lock( &a_rLockBuffer ) )
		{
			m_uiFlags |= FLAG_LOCKED;
			return TTRUE;
		}

		return TFALSE;
	}

	void Unlock( TUINT32 a_uiNumVertices )
	{
		TASSERT( 0 != ( m_uiFlags & FLAG_LOCKED ) );

		if ( TINT16( a_uiNumVertices ) == -1 )
		{
			a_uiNumVertices = m_pVertexPool->GetNumVertices();
		}

		m_pVertexPool->Unlock( a_uiNumVertices );
		m_uiFlags &= ~FLAG_LOCKED;
	}

public:
	TUINT32                              m_uiFlags;
	TUINT16                              m_uiMaxVertices;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	SubMesh*                             m_pSubMeshes;
};

class AWorldShader : public Toshi::TShader
{
public:
	virtual void        EnableRenderEnvMap( TBOOL a_bEnable )    = 0;
	virtual TBOOL       IsHighEndSkinning()                      = 0;
	virtual void        EnableHighEndSkinning( TBOOL a_bEnable ) = 0;
	virtual TBOOL       IsCapableHighEndSkinning()               = 0;
	virtual TBOOL       IsLightScattering()                      = 0;
	virtual void        SetLightScattering( TBOOL a_bEnable )    = 0;
	virtual void*       CreateMaterial( const TCHAR* a_szName )  = 0;
	virtual AWorldMesh* CreateMesh( const TCHAR* a_szName )      = 0;
};
