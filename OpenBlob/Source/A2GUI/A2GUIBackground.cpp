#include "pch.h"
#include "A2GUIBackground.h"
#include "Toshi/Render/TAssetInit.h"

using namespace Toshi;

void A2GUIBackground::Create(const char* fileName)
{
	Destroy();
	TStringManager::String8Copy(m_pFileName, fileName, MAX_FILENAME_LENGTH);
	LoadMaterialLibrary(fileName);
	SetupBackground();
}

void A2GUIBackground::LoadMaterialLibrary(const char* fileName)
{
	TString8 sixTeenStr = TString8::TString8("16");
	TString8 fileNameStr = TString8::TString8(fileName);

	char matlibPath[MAX_FILEPATH_LENGTH];

	TStringManager::String8Format(matlibPath, MAX_FILEPATH_LENGTH, "data/matlibs/%s.trb", fileNameStr);

	if (m_pTrb == TNULL)
	{
		m_pTrb = new TTRB();
	}

	TTRB::ERROR eRet = m_pTrb->Load(matlibPath);
	TASSERT(TTRB::ERROR_OK == eRet);
	TAssetInit::InitAssets(*m_pTrb, true, false);
}

void A2GUIBackground::SetupBackground()
{
	float width, height;
	TString8 str;

	auto gui = T2GUI::GetSingletonWeak();

	gui->GetRootElement()->GetDimensions(width, height);
	SetDimensions(width, height);

	uint32_t textureWidths[MAX_TILES];
	uint32_t textureHeights[MAX_TILES];

	for (size_t i = 0; i < MAX_TILES; i++)
	{
		str = str.Format("%s_%d.tga", m_pFileName, i+1);
		T2GUIMaterial* mat = T2GUI::CreateMaterial(str);
		textureWidths[i] = mat->GetTexture()->GetWidth();
		textureHeights[i] = mat->GetTexture()->GetHeight();
		T2GUI::DestroyMaterial(mat);
	}

	int iTile = 0;

	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < MAX_TILES / 2; j++)
		{
			TASSERT(iTile < MAX_TILES);

			str = str.Format("%s_%d.tga", m_pFileName, iTile + 1);

			TASSERT(TNULL == m_Poly[iTile]);

			m_Poly[iTile] = new T2GUIPolygon();
			m_Poly[iTile]->Create(4);
			m_Poly[iTile]->m_Anchor = T2GUIElement::Anchor::MiddleCenter;

			T2GUIMaterial* mat = T2GUI::CreateMaterial(str);
			m_Poly[iTile]->m_pMaterial = mat;

			TVector2 vec[4] = {
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 0.0f, 1.0f },
				{ 1.0f, 1.0f },
			};

			TVector2 vec2[4] = {
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 0.0f, 1.0f },
				{ 1.0f, 1.0f },
			};

			m_Poly[iTile]->m_pUnk = vec;
			m_Poly[iTile]->m_pUnk2 = vec2;

			AddChildTail(m_Poly[iTile]);
			iTile++;
		}
	}

	
}

void A2GUIBackground::Destroy()
{
	
	for (size_t i = 0; i < MAX_TILES; i++)
	{
		if (m_Poly[i] != TNULL)
		{
			T2GUI::DestroyMaterial(m_Poly[i]->m_pMaterial);
		}
	}

	if (m_pTrb != TNULL)
	{
		Toshi::TAssetInit::DeinitAssets(*m_pTrb);
		m_pTrb->~TTRB();
		m_pTrb = TNULL;
	}
}
