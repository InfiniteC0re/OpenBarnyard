#include "pch.h"
#include "ARenderer.h"
#include "AppBoot.h"
#include "Movie/AMoviePlayer.h"
#include "A2GUI/A2GUIRenderer_DX11.h"
#include "GameInterface/AFrontEndMovieState.h"
#include "AXYZViewportManager.h"

#include <Platform/Windows/DX11/TRender_DX11.h>
#include <Platform/Windows/DX11/TRenderContext_DX11.h>
#include <Platform/Windows/DX11/TPrimShader_DX11.h>
#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Shaders/SysShader/TSysShaderHAL.h>
#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Render/TOrderTable.h>

Toshi::TTRB ARenderer::s_BootAssetsTRB = Toshi::TTRB();
Toshi::THPTimer ARenderer::s_timer = Toshi::THPTimer();

static void MainScene(float deltaTime, void* pCameraObject)
{
	
}

ARenderer::ARenderer()
{
	// 005ed0e0
}

ARenderer::~ARenderer()
{
	// 005f0140
}

void ARenderer::Update(float deltaTime)
{
	TIMPLEMENT();
	auto pRender = Toshi::TRenderDX11::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	bool bRenderWorld = AApplication::g_oTheApp.ShouldRenderWorld();
	bool bRenderMovie = TFALSE;

	AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

	if (pMoviePlayer != TNULL && pMoviePlayer->IsMoviePlaying())
	{
		bRenderMovie = TTRUE;
	}

	
	pRender->Update(deltaTime);
	pRender->BeginScene();
	
	if (bRenderMovie)
	{
		pMoviePlayer->OnRender(deltaTime);
	}
	else if (bRenderWorld)
	{
		RenderMainScene(deltaTime, m_pViewport, TNULL, TNULL, MainScene, true);
	}

	RenderGUI(false);
	auto pGameStateController = AGameStateController::GetSingletonWeak();
	auto pGameState = pGameStateController->GetCurrentGameState();

	if (TFALSE == pGameState->GetClass()->IsA(TGetClass(AFrontEndMovieState)))
	{
		ID3D11ShaderResourceView* pShaderResourceView = (pDisplayParams->MultisampleQualityLevel < 2) ? pRender->m_SRView1 : pRender->m_SRView2;
		pRender->m_pFXAA->Render(pShaderResourceView);
	}

	s_timer.Update();
	m_fUpdateTime = s_timer.GetDelta();

	//TOSHI_INFO("Update time of ARenderer was: {}", m_fUpdateTime);

	s_timer.Reset();

	pRender->EndScene();
}

bool ARenderer::CreateInterface()
{
	// 005ed3b0
	Toshi::TOrderTable::CreateStaticData(4500, 2500);
	return CreateTRender();
}

bool ARenderer::CreateTRender()
{
	// 005f0780
	auto renderer = new Toshi::TRenderDX11();
	renderer->Create();

	Toshi::TRender::DisplayParams displayParams;

	renderer->BuildAdapterDatabase();
	auto adapterList = renderer->GetAdapterList();
	
	auto adapter = adapterList->Head()->As<Toshi::TD3DAdapter>();
	auto mode = adapter->GetMode();
	
	int width = 1280;
	int height = 720;

	if (AApplication::g_oTheApp.m_Width == -1)
	{
		if (AApplication::g_oTheApp.m_Width != -1)
		{
			MessageBoxA(TNULL, "You should specify both width and heigth or neither", "Invalid args", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			return false;
		}

		if (!AApplication::g_oTheApp.m_bUseDefaultHeightWidth)
		{
			width = mode->GetWidth();
			height = mode->GetHeight();
		}

		displayParams.Unk3 = 32;
		displayParams.Unk4 = 3;
		displayParams.Unk5 = AApplication::g_oTheApp.m_bUseDefaultHeightWidth;
		displayParams.IsFullscreen = AApplication::g_oTheApp.m_bIsFullscreen;
		displayParams.MultisampleQualityLevel = 1;
		displayParams.Width = width;
		displayParams.Height = height;
		renderer->CreateDisplay(&displayParams);

		return true;
	}
	else
	{
		if (AApplication::g_oTheApp.m_Height == -1)
		{
			MessageBoxA(TNULL, "You should specify both width and heigth or neither", "Invalid args", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
			return false;
		}

		if (!AApplication::g_oTheApp.m_bUseDefaultHeightWidth)
		{
			TTODO("Is mode valid?");

			if (AApplication::g_oTheApp.m_Width > 1280 && AApplication::g_oTheApp.m_Height > 720)
			{
				MessageBoxA(TNULL, "The specified width and heith must be greater than 1280x720 and match a valid mode", "Invalid args", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}
			displayParams.Unk3 = 32;
			displayParams.Unk4 = 3;
			displayParams.Unk5 = AApplication::g_oTheApp.m_bUseDefaultHeightWidth;
			displayParams.IsFullscreen = AApplication::g_oTheApp.m_bIsFullscreen;
			displayParams.MultisampleQualityLevel = 1;
			displayParams.Width = AApplication::g_oTheApp.m_Width;
			displayParams.Height = AApplication::g_oTheApp.m_Height;
			renderer->CreateDisplay(&displayParams);
			return true;
		}
		else
		{
			if (AApplication::g_oTheApp.m_Width <= 1280
				&& AApplication::g_oTheApp.m_Height <= 720
				&& mode->GetHeight() < AApplication::g_oTheApp.m_Height
				&& mode->GetWidth() < AApplication::g_oTheApp.m_Width)
			{
				char formattedString[512];
				Toshi::T2String8::Format(formattedString, "The specified width and heith must be between 1280x720 and the current desktop resolution (%dx%d)", mode->GetWidth(), mode->GetHeight());
				MessageBoxA(TNULL, formattedString, "Invalid args", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
				return false;
			}

			displayParams.Unk3 = 32;
			displayParams.Unk4 = 3;
			displayParams.Unk5 = AApplication::g_oTheApp.m_bUseDefaultHeightWidth;
			displayParams.IsFullscreen = AApplication::g_oTheApp.m_bIsFullscreen;
			displayParams.MultisampleQualityLevel = 1;
			displayParams.Width = width;
			displayParams.Height = height;
			renderer->CreateDisplay(&displayParams);
			return true;
		}
	}
}

void ARenderer::Create()
{
	TIMPLEMENT();

	auto error = s_BootAssetsTRB.Load("Data\\BootAssets\\BootAssets.trb");
	Toshi::TPrimShader::CreateSingleton();

	if (error == Toshi::TTRB::ERROR_OK)
	{
		Toshi::TAssetInit::InitAssets(s_BootAssetsTRB, true, false);
	}

	// Create SysShader
	Toshi::TSysShaderHAL::OrderTablePriorities priorities = {
		100,
		5220,
		5222,
		5225,
		5215,
		500,
		5219
	};

	auto pSysShader = Toshi::TSysShaderHAL::CreateSingleton<Toshi::TSysShaderHAL>(&priorities);
	pSysShader->Create();

	CreateMainViewport();

	AXYZViewportManager* vpMgr = new AXYZViewportManager();

	float width, height;

	Toshi::T2GUI::GetSingletonWeak()->GetRootElement()->GetDimensions(width, height);

	m_pRectangle = new Toshi::T2GUIRectangle();
	
	m_pRectangle->Create(width, height);
	m_pRectangle->SetColour(0xFF000000);
	m_pRectangle->SetAlpha(0);
	m_pRectangle->SetVisible(true);
}

void ARenderer::SetBackgroundColour(uint32_t r, uint32_t g, uint32_t b)
{
	if (m_pViewport != TNULL)
	{
		m_pViewport->SetBackgroundColor(r & 0xFF, g & 0xFF, b & 0xFF, 255);
	}

	m_BackgroundColor = ((r & 0xff) << 8 | g & 0xff) << 8 | b & 0xff;
}

void ARenderer::RenderGUI(bool allowBackgroundClear)
{
	TIMPLEMENT();

	m_pViewport->AllowBackgroundClear(allowBackgroundClear);
	m_pViewport->Begin();
	Toshi::T2GUI::GetSingletonWeak()->Render();
	m_pViewport->End();
}

void ARenderer::RenderMainScene(float deltaTime, Toshi::TViewport* pViewport, void* unk2, void* pCameraObject, t_MainScene mainSceneCb, bool allowBackgroundClear)
{
	TIMPLEMENT();
	auto pRender = Toshi::TRenderDX11::Interface();
	
	pViewport->AllowDepthClear(true);

	auto pOriginalContext = pRender->GetCurrentRenderContext();
	pRender->SetCurrentRenderContext(pViewport->GetRenderContext());

	pViewport->AllowBackgroundClear(allowBackgroundClear);
	pViewport->Begin();
	// ...
	pRender->m_pGlow->Render(pRender->m_SRView2, TNULL);
	mainSceneCb(deltaTime, pCameraObject);
	pViewport->End();
	// ...

	pRender->FlushShaders();
	pRender->SetCurrentRenderContext(pOriginalContext);
}

void ARenderer::CreateMainViewport()
{
	auto renderer = Toshi::TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

	m_pViewport = new Toshi::TViewport();
	m_pViewport->SetMemoryAllocatorBlock(AMemory::ms_apMemoryBlocks[AMemory::POOL_FrequentAllocations]);
	m_pViewport->SetX(0.0f);
	m_pViewport->SetY(0.0f);
	m_pViewport->SetWidth((float)pDisplayParams->Width);
	m_pViewport->SetHeight((float)pDisplayParams->Height);
	m_pViewport->SetMinZ(0.2f);
	m_pViewport->SetMaxZ(1.0f);
	m_pViewport->AllowBackgroundClear(true);
	m_pViewport->AllowDepthClear(true);
	m_pViewport->Enable(true);
	m_pViewport->EnableDefaultBeginRender(true);
	m_pViewport->SetBackgroundColor(0, 0, 0, 255);
	renderer->m_pGlow->SetDist(1.5f);
	renderer->m_pGlow->SetIntensity(4.0f);
}

void ARenderer::CreateTRenderResources()
{
	// Since this is deprecated and factory resource classes don't even exist, TSysShader is not used too
	TASSERT(TFALSE, "Deprecated in de blob");
}
