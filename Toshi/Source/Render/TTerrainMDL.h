#pragma once
#include <Math/TVector2.h>
#include <Math/TVector3.h>
#include <Math/TSphere.h>

namespace Toshi {

	namespace TTerrainMDL {

		struct WorldVertex
		{
			TVector3 Position;
			TVector3 Normal;
			TVector3 Color;
			TVector2 UV;
		};

		struct Mesh
		{
			TUINT32 m_Unk;
			TUINT32 m_uiNumIndices;
			TUINT32 m_uiNumVertices1;
			TUINT32 m_uiNumVertices2;
			TCHAR* m_szMaterialName;
			WorldVertex* m_pVertices;
			TUINT16* m_pIndices;
		};

		struct MeshBounding
		{
			TSphere m_BoundingSphere;
			Mesh* m_pMesh;
		};

		struct RenderGroupData
		{
			TUINT32 m_uiNumMeshes;

			TUINT16 GetMeshIndex(TUINT32 a_uiIndex)
			{
				TASSERT(a_uiIndex < m_uiNumMeshes);
				return *((TUINT16*)(this + 1) + a_uiIndex);
			}
		};

		struct RenderGroup
		{
			TSphere m_BoundingSphere;
			RenderGroup* m_pNextGroup;

			RenderGroupData* GetData()
			{
				TASSERT(m_pNextGroup == TNULL);
				return (RenderGroupData*)(this + 1);
			}

			RenderGroup* GetSubGroup()
			{
				TASSERT(m_pNextGroup != TNULL);
				return (RenderGroup*)(this + 1);
			}
		};

		struct Model
		{
			TCHAR m_MetaData[132];
			TUINT32 m_uiNumMeshes;
			MeshBounding** m_ppMeshBoundings;
			RenderGroup* m_pRenderGroups;
		};

		struct Entry
		{
			TUINT32 m_uiNumModels;
			Model** m_ppModels;
		};

		struct TRBHeader
		{
			TUINT32 m_uiNumEntries;
			Entry** m_ppEntries;
		};

	}

}