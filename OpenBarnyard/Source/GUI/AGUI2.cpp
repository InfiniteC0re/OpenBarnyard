#include "pch.h"
#include "AGUI2.h"
#include "Assets/AAssetLoader.h"
#include "AGUI2FontManager.h"
#include "AGUI2TextureSectionManager.h"
#include "Assets/AMaterialLibraryManager.h"

#include <Toshi/Core/TSystem.h>
#include TOSHI_MULTIRENDER(AGUI2Renderer)

TOSHI_NAMESPACE_USING

AGUI2::AGUI2()
{
	m_pRootElement = TNULL;
	m_bShowMouseCursor = TFALSE;

	m_bShowMemStatsInfo = TTRUE;
	m_bShowFPSInfo = TTRUE;
	m_bShowPlayerInfo = TFALSE;
	m_bShowTexturesInfo = TTRUE;
}

TBOOL AGUI2::OnCreate()
{
	CreateContext();
	ms_pCurrentContext->SetRenderer(new AGUI2RendererDX8);

	m_pRootElement = ms_pCurrentContext->GetRootElement();
	m_pRootElement->SetDimensions(800.0f, 600.0f);
	m_pRootElement->SetPostRenderCallback(MainPostRenderCallback);

	AGUI2FontManager::Open("data/gui/fonts.trb");
	AGUI2TextureSectionManager::Open("data/gui/texsec.trb", AAssetLoader::GetAssetTRB(AAssetType_Startup));
	AGUI2TextureSectionManager::CreateMaterials();

	auto pDebugCanvas = ms_pCurrentContext->GetDebugCanvas();
	auto pFont = AGUI2FontManager::FindFont("Rekord18");

	// FPS stats
	m_oFPS.Create(pFont, 600.0f);
	m_oFPS.SetAttachment(AGUI2Element::Anchor_TopLeft, AGUI2Element::Anchor_TopLeft);
	m_oFPS.SetTextAlign(AGUI2Font::TextAlign_Left);
	m_oFPS.SetTransform(0.0f, 0.0f);

	if (m_bShowFPSInfo) m_oFPS.Show();
	else m_oFPS.Hide();
	pDebugCanvas->AddChildTail(m_oFPS);

	// Player stats
	m_oPlayerInfo.Create(pFont, 600.0f);
	m_oPlayerInfo.SetAttachment(AGUI2Element::Anchor_TopLeft, AGUI2Element::Anchor_TopLeft);
	m_oPlayerInfo.SetTextAlign(AGUI2Font::TextAlign_Left);
	m_oPlayerInfo.SetTransform(0.0f, 24.0f);

	if (m_bShowPlayerInfo) m_oPlayerInfo.Show();
	else m_oPlayerInfo.Hide();
	pDebugCanvas->AddChildTail(m_oPlayerInfo);

	// Textures stats
	m_oTexturesInfo.Create(pFont, 600.0f);
	m_oTexturesInfo.SetAttachment(AGUI2Element::Anchor_TopLeft, AGUI2Element::Anchor_TopLeft);
	m_oTexturesInfo.SetTextAlign(AGUI2Font::TextAlign_Left);
	m_oTexturesInfo.SetTransform(0.0f, 48.0f);

	if (m_bShowTexturesInfo) m_oTexturesInfo.Show();
	else m_oTexturesInfo.Hide();
	pDebugCanvas->AddChildTail(m_oTexturesInfo);

	// Memory stats
	m_oMemStats.Create(pFont, 600.0f);
	m_oMemStats.SetAttachment(AGUI2Element::Anchor_TopLeft, AGUI2Element::Anchor_TopLeft);
	m_oMemStats.SetTextAlign(AGUI2Font::TextAlign_Left);
	m_oMemStats.SetTransform(0.0f, 72.0f);

	if (m_bShowMemStatsInfo) m_oMemStats.Show();
	else m_oMemStats.Hide();
	pDebugCanvas->AddChildTail(m_oMemStats);

	// Mouse cursor
	m_oMouseCursor.Create("Pointer_Up", "Pointer_Down", 0.5f);

	return TTRUE;
}

TBOOL AGUI2::OnUpdate(TFLOAT a_fDeltaTime)
{
	if (m_bShowMemStatsInfo)
	{
		static TFLOAT s_UpdateTimer = 10.0f;

		s_UpdateTimer += a_fDeltaTime;

		if (s_UpdateTimer >= 1.0f)
		{
			Toshi::TMemory::MemInfo memInfo;
			Toshi::TMemory::GetSingleton()->GetMemInfo(memInfo, TNULL);

			Toshi::TStringManager::String16Format(
				m_wszMemStats,
				sizeof(m_wszMemStats),
				L"Mem Used: %u, Free: %u\nHoles: %u, Largest: %u",
				memInfo.m_uiLogicTotalUsed,
				memInfo.m_uiLogicTotalFree,
				memInfo.m_iNumHoles,
				memInfo.m_uiLargestHole
			);

			m_oMemStats.SetText(m_wszMemStats);
			s_UpdateTimer = 0.0f;
		}
	}

	if (m_bShowFPSInfo)
	{
		static TFLOAT s_FPS = 0.0f;
		static TFLOAT s_UpdateTimer = 0.0f;
		s_UpdateTimer += a_fDeltaTime;

		if (s_UpdateTimer > 0.1f)
		{
			s_FPS = Toshi::TSystemManager::GetSingleton()->GetAverageFps();
			s_UpdateTimer = 0.0f;
		}

		Toshi::TStringManager::String16Format(
			m_wszFPS,
			sizeof(m_wszFPS),
			L"FPS: %0.02f",
			s_FPS
		);

		m_oFPS.SetText(m_wszFPS);

		if (s_FPS >= 15.0f)
		{
			m_oFPS.SetColour(s_FPS < 20.0f ? 0xffffff00 : 0xffffffff);
		}
		else
		{
			m_oFPS.SetColour(0xffff0000);
		}
	}

	if (m_bShowPlayerInfo)
	{
		TTODO("Debug info about player");
	}

	if (m_bShowTexturesInfo)
	{
		auto iNumFreeTextures = AMaterialLibraryManager::GetSingleton()->GetNumFreeTextures();
		auto iNumUsedTextures = AMaterialLibraryManager::GetSingleton()->GetNumUsedTextures();

		Toshi::TStringManager::String16Format(
			m_wszTexturesInfo,
			sizeof(m_wszTexturesInfo),
			L"Textures: (F:%d,U:%d)/%d",
			iNumFreeTextures,
			iNumUsedTextures,
			AMaterialLibraryManager::MAX_NUM_TEXTURES
		);

		m_oTexturesInfo.SetText(m_wszTexturesInfo);

		if (iNumFreeTextures >= 20)
		{
			m_oTexturesInfo.SetColour(iNumFreeTextures < 50 ? 0xffffff00 : 0xffffffff);
		}
		else
		{
			m_oTexturesInfo.SetColour(0xffff0000);
		}
	}

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
