#include "pch.h"
#include "AGUIMatlibPicture.h"
#include "AGUI2.h"
#include "Assets/AMaterialLibraryManager.h"
#include "Assets/AAssetLoader.h"

#include <Render/TRenderInterface.h>
#include <Render/TTextureFactory.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(AGUIMatlibPicture);

void AGUIMatlibPicture::Flush()
{
	if (m_iNumRects1 != 0)
	{
		if (m_pImages.GetArray() != TNULL)
		{
			for (TINT i = 0; i < m_iNumRects2; i++)
			{
				if (m_pImages[i].pTexture)
				{
					m_pImages[i].pTexture->DestroyResource();
					m_pImages[i].pTexture = TNULL;
				}

				AGUI2::GetRenderer()->DestroyMaterial(m_pImages[i].pMaterial);
				m_pImages[i].pMaterial = TNULL;
			}

			m_pImages.Destroy();

			m_iNumRects2 = 0;
			m_pRectangles.Destroy();

			m_iNumRects1 = 0;
			UnloadMatLib();
			m_bLoaded = TFALSE;
		}
	}
}

void AGUIMatlibPicture::Cache()
{
	TASSERT(m_bLoaded == TFALSE);

	if (m_iNumRects1 < 1)
	{
		if (!m_Name.IsEmpty())
		{
			m_iNumRects1 = 6;
			m_pRectangles.Create(6);

			m_iNumRects2 = 6;
			m_pImages.Create(6);
			LoadMatLib();

			auto pTexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TTextureFactory>(SYSRESOURCE_TEXTUREFACTORY);

			for (TINT i = 0; i < m_iNumRects2; i++)
			{
				TString8 texName;
				texName.Format("%s_%d.tga", m_Name.GetString8().GetString(), i + 1);

				auto iTexIndex = m_pMaterialLibrary->FindTextureIndex(texName);
				auto pTexture = pTexFactory->CreateFromT2Texture(m_pMaterialLibrary->GetTexture(iTexIndex)->pTexture);

				m_pImages[i].pTexture = pTexture;
				m_pImages[i].pMaterial = AGUI2::GetRenderer()->CreateMaterial(pTexture);
				m_pImages[i].pMaterial->SetBlendState(0);
				m_pImages[i].pMaterial->SetTextureAddress(1);
			}

			for (TINT i = 0; i < m_iNumRects1; i++)
			{
				auto pRect = &m_pRectangles[i];
				pRect->ppMaterial = &m_pImages[AGUIPicture::split_rules[i].iImageIndex].pMaterial;

				(pRect->Pos1).y = AGUIPicture::split_rules[i].Pos2.x;
				(pRect->Pos1).x = AGUIPicture::split_rules[i].Pos1.x;
				(pRect->Pos2).y = AGUIPicture::split_rules[i].Pos2.y;
				(pRect->Pos2).x = AGUIPicture::split_rules[i].Pos1.y;
				(pRect->UV1).y = AGUIPicture::split_rules[i].UV2.x;
				(pRect->UV1).x = AGUIPicture::split_rules[i].UV1.x;
				(pRect->UV2).y = AGUIPicture::split_rules[i].UV2.y;
				(pRect->UV2).x = AGUIPicture::split_rules[i].UV1.y;
			}

			m_bLoaded = TTRUE;
		}
	}
}

void AGUIMatlibPicture::Create(const TPString8& a_rName)
{
	if (m_Name != a_rName) Flush();
	m_Name = a_rName;
}

void AGUIMatlibPicture::LoadMatLib()
{
	TString8 format("16");

	TCHAR path[256];
	TStringManager::String8Format(path, sizeof(path), "data/matlibs/%s.ttl", m_Name.GetString8().GetString());
	
	m_pMaterialLibrary = AMaterialLibraryManager::List::GetSingleton()->CreateLibraryFromAsset(
		path,
		AAssetLoader::GetAssetTRB(AAssetType_AssetPack)
	);
}

void AGUIMatlibPicture::UnloadMatLib()
{
	TVALIDPTR(m_pMaterialLibrary);

	m_pMaterialLibrary->Destroy();
	m_pMaterialLibrary = TNULL;
	m_Name = TPString8();
}

AGUIMatlibPicture::AGUIMatlibPicture()
{

}

AGUIMatlibPicture::~AGUIMatlibPicture()
{
	Flush();
}