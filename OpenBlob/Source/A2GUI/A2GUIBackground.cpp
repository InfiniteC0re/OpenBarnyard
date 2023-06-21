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
	TAssetInit::InitAssets(*m_pTrb, TTRUE, TFALSE);
}

void A2GUIBackground::SetupBackground()
{
	auto gui = T2GUI::GetSingletonWeak();
	
	SetAnchor(Anchor::TopRight);
	SetPivot(Pivot::TopRight);

	float width, height;
	gui->GetRootElement()->GetDimensions(width, height);
	SetDimensions(width, height);

	uint32_t textureWidths[3];
	uint32_t textureHeights[2];

	char* tgaName = TStringManager::GetTempString8();
	
	for (size_t i = 0; i < MAX_TILES; i++)
	{
		T2String8::Format(tgaName, "%s_%d.tga", m_pFileName, i+1);
		T2GUIMaterial* pMaterial = T2GUI::CreateMaterial(tgaName);
		textureWidths[i % 3] = pMaterial->GetTexture()->GetWidth();
		textureHeights[i / 3] = pMaterial->GetTexture()->GetHeight();
		T2GUI::DestroyMaterial(pMaterial);
	}

	float backgroundWidth = 0.0f + textureWidths[0] + textureWidths[1] + textureWidths[2];
	float backgroundHeight = 0.0f + textureHeights[0] + textureHeights[1];

	int iTile = 0;
	int tileX = 0, tileY = 0;

	for (size_t i = 0; i < 2; i++)
	{
		int tileHeight = static_cast<int>((textureHeights[i] / backgroundHeight) * height);
		tileX = 0;

		for (size_t j = 0; j < MAX_TILES / 2; j++)
		{
			TASSERT(iTile < MAX_TILES);
			TASSERT(TNULL == m_Poly[iTile]);

			int tileWidth = TSTATICCAST(int, (textureWidths[j] / backgroundWidth) * width);
			auto& pTile = m_Poly[iTile];
			pTile = new T2GUIPolygon;

			pTile->Create(4);
			pTile->SetAnchor(Anchor::TopRight);
			pTile->SetPivot(Pivot::MiddleCenter);
			
			T2String8::Format(tgaName, "%s_%d.tga", m_pFileName, iTile + 1);
			T2GUIMaterial* pMaterial = T2GUI::CreateMaterial(tgaName);
			pMaterial->GetTexture()->SetWrap(D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP);
			pTile->SetMaterial(pMaterial);

			auto uv = pTile->GetUV();
			uv[0].x = 0.0;
			uv[0].y = 0.0;
			uv[1].x = 1.0;
			uv[1].y = 0.0;
			uv[2].x = 0.0;
			uv[2].y = 1.0;
			uv[3].x = 1.0;
			uv[3].y = 1.0;

			auto startX = tileX;
			auto endX = tileX + tileWidth;

			auto vertices = pTile->GetVertices();
			vertices[0].x = static_cast<float>(startX);
			vertices[0].y = (height - tileY) - s_OffsetY;
			vertices[1].x = static_cast<float>(endX);
			vertices[1].y = (height - tileY) - s_OffsetY;
			vertices[2].x = static_cast<float>(startX);
			vertices[2].y = (height - tileHeight) - tileY;
			vertices[3].x = static_cast<float>(endX);
			vertices[3].y = (height - tileHeight) - tileY;

			AddChildTail(pTile);
			tileX += tileWidth;

			iTile++;
		}

		tileY += tileHeight;
	}
}

void A2GUIBackground::Destroy()
{
	for (size_t i = 0; i < MAX_TILES; i++)
	{
		if (m_Poly[i] != TNULL)
		{
			delete m_Poly[i];
		}
	}

	if (m_pTrb != TNULL)
	{
		Toshi::TAssetInit::DeinitAssets(*m_pTrb);
		delete m_pTrb;
		m_pTrb = TNULL;
	}
}
