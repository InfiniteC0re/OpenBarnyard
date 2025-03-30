#pragma once
#include <Render/TMesh.h>
#include <Math/TVector2.h>
#include <Math/TVector3.h>
#include <Math/TPlane.h>
#include <Math/TSphere.h>
#include <Math/TMatrix44.h>

struct CullBox
{
	TFLOAT x;
	TFLOAT y;
	TFLOAT width;
	TFLOAT height;
};

struct CellSettings
{
	CellSettings* pNext;
	CullBox       oCullBox;
	TINT          Unknown;
};

struct CellNode
{
	TINT          iCellIndex;
	CellSettings* pCellSettings;
	TINT          Unknown;
};

struct FrustumPlane
{
	Toshi::TPlane oPlane;
	TUINT         uiClipFlag;
};

enum FrustumIntersectSphereResult
{
	FISR_ALL_VISIBLE,
	FISR_NOT_VISIBLE,
	FISR_PARTIALLY_VISIBLE,
};

class Frustum
{
public:
	Frustum();

	void                         InitReduce();
	FrustumIntersectSphereResult IntersectSphereReduce( const Toshi::TSphere& a_rSphere );

	void Transform( const Frustum& a_rFrustum, const Toshi::TMatrix44& a_rMatrix );

public:
	TUINT16        aUnknown[ 6 ];
	FrustumPlane   aFrustumPlanes[ 6 ];
	Toshi::TPlane* apActivePlanes[ 6 ];
	TINT           iActivePlaneCount;
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

	TUINT16& GetMeshIndex( TUINT32 a_uiIndex )
	{
		TASSERT( a_uiIndex < m_uiNumMeshes );
		return *( (TUINT16*)( this + 1 ) + a_uiIndex );
	}

	struct CellSphereTreeBranchNode* End()
	{
		return (struct CellSphereTreeBranchNode*)TAlignPointerUp( ( (TUINT16*)( this + 1 ) ) + m_uiNumMeshes );
	}
};

struct CellSphereTreeBranchNode
{
	Toshi::TSphere            m_BoundingSphere;
	CellSphereTreeBranchNode* m_pRight;

	TBOOL IsLeaf() const { return m_pRight == TNULL; }

	CellSphereTreeLeafNode* GetLeafNode()
	{
		TASSERT( IsLeaf() );
		return (CellSphereTreeLeafNode*)( this + 1 );
	}

	CellSphereTreeBranchNode* GetSubNode()
	{
		TASSERT( !IsLeaf() );
		return (CellSphereTreeBranchNode*)( this + 1 );
	}

	CellSphereTreeBranchNode* GetPrevNode()
	{
		return (CellSphereTreeBranchNode*)( this - 1 );
	}
};

struct Cell
{
	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_BUILDING = BITFLAG( 0 )
	};

	FLAGS                     uiFlags;
	TCHAR                     UNKNOWNDATA1[ 108 ];
	TINT                      m_iSomeCount;
	TCHAR                     UNKNOWNDATA2[ 12 ];
	CellNode*                 pNode;
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

struct RenderData
{
	Cell*    pCell;
	Frustum* pFrustum;
};
