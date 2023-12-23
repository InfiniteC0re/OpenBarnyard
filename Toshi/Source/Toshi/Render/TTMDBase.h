#pragma once

namespace Toshi {

	namespace TTMDBase {

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
			char m_szTKLName[16];
			TINT32 m_iTKeyCount;
			TINT32 m_iQKeyCount;
			TINT32 m_iSKeyCount;
			TINT32 m_iTBaseIndex;
			TINT32 m_iQBaseIndex;
			TINT32 m_iSBaseIndex;
		};

		struct Material
		{
			char m_szMatName[104];
			char m_szTextureFile[192];
		};

		struct Materials
		{
			TUINT32 m_uiZero1;
			TUINT32 m_uiZero2;
			TUINT32 m_uiNumMaterials;
			TUINT32 m_uiSectionSize;

			Material* GetMaterial(TUINT a_uiIndex)
			{
				TASSERT(a_uiIndex < m_uiNumMaterials);
				return TREINTERPRETCAST(Material*, this + 1) + a_uiIndex;
			}
		};

		struct CollisionModel
		{
			TUINT32 m_Unk1;
			TVector3* m_pVertices;
			TUINT32 m_uiNumVertices;
			TUINT16* m_pIndices;
			TUINT32 m_uiNumIndices;
			TUINT32 m_uiNumCollTypes;
			char** m_pszCollName;
			TUINT32 m_Unk3;
		};

		struct Collision
		{
			TINT m_iNumModels;
			CollisionModel* m_pModels;
		};

		struct Mesh
		{

		};
	}

}