#include "pch.h"
#include "AImGuiRenderer.h"
#include "ImGui/AImGui.h"

AImGuiRenderer::AImGuiRenderer()
{
}

TBOOL AImGuiRenderer::CreateTRender()
{
	auto pRender = new Toshi::TRENDER();
	TBOOL bCreatedRender = pRender->Create();

	Toshi::TRender::DisplayParams displayParams;
	displayParams.Unk3 = 32;
	displayParams.Unk4 = 3;
	displayParams.Unk5 = TFALSE;
	displayParams.IsFullscreen = TFALSE;
	displayParams.MultisampleQualityLevel = 1;
	displayParams.Width = 1280;
	displayParams.Height = 720;
	pRender->CreateDisplay(&displayParams);
	
	return bCreatedRender && CreateMainViewport() && CreateImGui();
}

TBOOL AImGuiRenderer::CreateMainViewport()
{
	auto pRender = Toshi::TRENDER::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	m_pViewport = new Toshi::TViewport;
	m_pViewport->SetX(0.0f);
	m_pViewport->SetY(0.0f);
	m_pViewport->SetWidth((float)pDisplayParams->Width);
	m_pViewport->SetHeight((float)pDisplayParams->Height);
	m_pViewport->SetMinZ(0.2f);
	m_pViewport->SetMaxZ(1.0f);
	m_pViewport->AllowBackgroundClear(TTRUE);
	m_pViewport->AllowDepthClear(TTRUE);
	m_pViewport->Enable(TTRUE);
	m_pViewport->EnableDefaultBeginRender(TTRUE);
	m_pViewport->SetBackgroundColor(255, 255, 255, 255);

	return TTRUE;
}

TBOOL AImGuiRenderer::CreateImGui()
{
	return AImGui::CreateSingleton() != TNULL;
}

void AImGuiRenderer::Update(float a_fDeltaTime)
{
	auto pRender = Toshi::TRENDER::Interface();

	pRender->Update(a_fDeltaTime);
	pRender->BeginScene();

	m_pViewport->Begin();
	AImGui::GetSingleton()->Render();
	m_pViewport->End();
	
	pRender->EndScene();
}

