#include "pch.h"
#include "AGUI2.h"
#include "Assets/AAssetLoader.h"

TOSHI_NAMESPACE_USING

AGUI2::AGUI2()
{
	m_pRootElement = TNULL;
	m_bShowMouseCursor = TFALSE;
	m_bShowMemStatsInfo = TFALSE;
	m_bShowFPSInfo = TFALSE;
	m_bShowPlayerInfo = TFALSE;
	m_bShowTexturesInfo = TFALSE;
}

TBOOL AGUI2::OnCreate()
{
	TIMPLEMENT();

	CreateContext();
	m_pRootElement = ms_pCurrentContext->GetRootElement();
	m_pRootElement->SetDimensions(800.0f, 600.0f);
	m_pRootElement->SetPostRenderCallback(MainPostRenderCallback);

	AGUI2TextureSectionManager::Open("data/gui/texsec.trb", AAssetLoader::GetAssetTRB(AAssetLoader::AssetType_Startup));
	AGUI2TextureSectionManager::CreateMaterials();

	return TTRUE;
}

TBOOL AGUI2::OnUpdate(float a_fDeltaTime)
{
	TIMPLEMENT();
	ms_pCurrentContext->Tick(a_fDeltaTime);

	return TTRUE;
}

void AGUI2::CreateContext()
{
	ms_pCurrentContext = new AGUI2Context;
}

AGUI2TextureSection* AGUI2::GetTextureSection(const char* a_szSectionName)
{
	auto pDefblock = AGUI2TextureSectionManager::g_defblock;
	auto pSections = AGUI2TextureSectionManager::g_sections;

	for (size_t i = 0; i < pDefblock->m_uiNumSections; i++)
	{
		auto pSection = &pSections[i];

		if (TStringManager::String8Compare(pSection->GetName(), a_szSectionName) == 0)
			return pSection;
	}

	return TNULL;
}

AGUI2Renderer* AGUI2::GetRenderer()
{
	TASSERT(ms_pCurrentContext != TNULL);
	return ms_pCurrentContext->GetRenderer();
}

void AGUI2::MainPostRenderCallback()
{
	if (GetSingleton()->m_bShowMouseCursor)
	{
		TTODO("Render in-game mouse cursor");
	}
}
