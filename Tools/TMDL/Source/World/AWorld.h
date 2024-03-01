#pragma once
#include <Render/TMesh.h>
#include <Math/TVector2.h>
#include <Math/TVector3.h>
#include <Math/TPlane.h>
#include <Math/TSphere.h>

struct CullBox
{
	TFLOAT x;
	TFLOAT y;
	TFLOAT width;
	TFLOAT height;
};

struct Frustum
{
	Toshi::TPlane aPlanes[6];
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
	TUINT32 uiNumIndices;
	TUINT32 uiNumVertices1;
	TUINT16 uiNumVertices2;
	TCHAR* szMaterialName;
	WorldVertex* pVertices;
	TUINT16* pIndices;
};

struct CellMeshSphere
{
	Toshi::TSphere m_BoundingSphere;
	CellMesh* m_pCellMesh;
};

struct CellSphereTreeLeafNode
{
	TUINT32 m_uiNumMeshes;

	TUINT16 GetMeshIndex(TUINT32 a_uiIndex)
	{
		TASSERT(a_uiIndex < m_uiNumMeshes);
		return *((TUINT16*)(this + 1) + a_uiIndex);
	}
};

struct CellSphereTreeBranchNode
{
	Toshi::TSphere m_BoundingSphere;
	CellSphereTreeBranchNode* m_pRight;

	TBOOL IsLeaf() const { return m_pRight == TNULL; }

	CellSphereTreeLeafNode* GetLeafNode()
	{
		TASSERT(m_pRight == TNULL);
		return (CellSphereTreeLeafNode*)(this + 1);
	}

	CellSphereTreeBranchNode* GetSubNode()
	{
		TASSERT(m_pRight != TNULL);
		return (CellSphereTreeBranchNode*)(this + 1);
	}
};

struct Cell
{
	TCHAR m_MetaData[128];
	TINT m_Unknown;
	TUINT32 uiNumMeshes;
	CellMeshSphere** ppCellMeshSpheres;
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