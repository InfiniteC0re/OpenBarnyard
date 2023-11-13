#include "pch.h"
#include "AGUI2.h"
#include "Assets/AAssetLoader.h"
#include "AGUI2FontManager.h"
#include "AGUI2TextureSectionManager.h"

#include TOSHI_MULTIRENDER(AGUI2Renderer)

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
	ms_pCurrentContext->SetRenderer(new AGUI2RendererDX8);

	m_pRootElement = ms_pCurrentContext->GetRootElement();
	m_pRootElement->SetDimensions(800.0f, 600.0f);
	m_pRootElement->SetPostRenderCallback(MainPostRenderCallback);

	AGUI2FontManager::Open("data/gui/fonts.trb");
	AGUI2TextureSectionManager::Open("data/gui/texsec.trb", AAssetLoader::GetAssetTRB(AAssetLoader::AssetType_Startup));
	AGUI2TextureSectionManager::CreateMaterials();

	m_oMouseCursor.Create("Pointer_Up", "Pointer_Down", 0.5f);

	m_TestTextBox.Create(AGUI2FontManager::FindFont("Rekord26"), 160.0f);
	m_TestTextBox.SetAttachment(AGUI2Element::Pivot_BottomCenter, AGUI2Element::Pivot_BottomCenter);
	m_TestTextBox.SetText(L"Loading...");
	m_TestTextBox.SetTransform(0.0f, m_pRootElement->GetHeight() * -0.08f);
	m_pRootElement->AddChildTail(&m_TestTextBox);

	return TTRUE;
}

TBOOL AGUI2::OnUpdate(float a_fDeltaTime)
{
	TIMPLEMENT();

	m_oMouseCursor.Update();
	ms_pCurrentContext->Tick(a_fDeltaTime);

	return TTRUE;
}

void AGUI2::GetDimensions(TFLOAT& a_rWidth, TFLOAT& a_rHeight)
{
	m_pRootElement->GetDimensions(a_rWidth, a_rHeight);
}

void AGUI2::CreateContext()
{
	ms_pCurrentContext = new AGUI2Context;
}

AGUI2Renderer* AGUI2::GetRenderer()
{
	TASSERT(ms_pCurrentContext != TNULL);
	return ms_pCurrentContext->GetRenderer();
}

void AGUI2::MainPostRenderCallback()
{
	// FIXME: Render only when cursor should be visible
	if (!AGUI2::GetSingleton()->IsCursorVisible())
	{
		auto& mouseCursor = AGUI2::GetSingleton()->m_oMouseCursor;

		mouseCursor.PreRender();
		mouseCursor.Render();
		mouseCursor.PostRender();
	}
}
