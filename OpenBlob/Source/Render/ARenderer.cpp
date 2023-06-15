#include "pch.h"
#include "ARenderer.h"
#include "AppBoot.h"
#include "AXYZViewportManager.h"
#include "ATestModel.h"
#include "Movie/AMoviePlayer.h"
#include "A2GUI/A2GUIRenderer_DX11.h"
#include "GameInterface/AFrontEndMovieState.h"
#include "Cameras/ACameraManager.h"

#include <Platform/Windows/DX11/TRender_DX11.h>
#include <Platform/Windows/DX11/TRenderContext_DX11.h>
#include <Platform/Windows/DX11/TPrimShader_DX11.h>
#include <Toshi/Render/TAssetInit.h>
#include <Toshi/Shaders/SysShader/TSysShaderHAL.h>
#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Render/TOrderTable.h>

using namespace Toshi;

TTRB ARenderer::s_BootAssetsTRB = TTRB();
THPTimer ARenderer::s_FrameTimer = THPTimer();

static void MainScene(float deltaTime, void* pCameraObject)
{
	
}

ARenderer::ARenderer()
{
	// 005ed0e0
	TIMPLEMENT();
	m_bRenderGUI = TTRUE;
	m_bIsLoadingScreen = TFALSE;
	m_BackgroundColor = TCOLOR(0, 0, 0);
}

ARenderer::~ARenderer()
{
	TIMPLEMENT();
	// 005f0140
}

void ARenderer::Update(float deltaTime)
{
	TIMPLEMENT();
	auto pRender = TRenderDX11::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	AMoviePlayer* pMoviePlayer = AMoviePlayer::GetSingletonWeak();

	TBOOL bRenderWorld = AApplication::g_oTheApp.ShouldRenderWorld();
	TBOOL bRenderMovie = pMoviePlayer != TNULL && pMoviePlayer->IsMoviePlaying();
	
	pRender->Update(deltaTime);
	pRender->BeginScene();
	
	if (bRenderMovie)
	{
		pMoviePlayer->OnRender(deltaTime);
	}
	else if (bRenderWorld)
	{
		m_pViewportManager->SetCameraUsed(AXYZViewportManager::VIEWPORT_FullScreen, TFALSE);
		m_pViewportManager->SetCameraUsed(AXYZViewportManager::VIEWPORT_1P1_HORIZONTAL_TOP, TTRUE);

		GetAppCamera();
		InitialiseViewPort();

		for (int i = AXYZViewportManager::VIEWPORT_Count - 1; i >= 0; i--)
		{
			if (m_pViewportManager->IsViewportCameraUsed(i))
			{
				m_pViewportManager->SetRenderingViewportIndex(i);

				auto pViewport = m_pViewportManager->GetViewport(i);
				auto pCameraObject = m_pViewportManager->GetViewportCameraObject(i);
				auto pCamera = m_pViewportManager->GetViewportCamera(i);

				if (pCamera == TNULL)
					pCamera = ACameraManager::GetSingletonWeak()->GetCurrentCamera();

				RenderMainScene(deltaTime, pViewport, pCameraObject, pCamera, MainScene, i == AXYZViewportManager::VIEWPORT_FullScreen);
			}
		}

		m_pViewportManager->SetRenderingViewportIndex(-1);
	}
	else if (m_bIsLoadingScreen)
	{
		GetAppCamera();
		InitialiseViewPort();
		RenderLoadingScreen(deltaTime);
	}

	RenderGUI(false);
	auto pGameStateController = AGameStateController::GetSingletonWeak();
	auto pGameState = pGameStateController->GetCurrentGameState();

	if (TFALSE == pGameState->GetClass()->IsA(TGetClass(AFrontEndMovieState)))
	{
		ID3D11ShaderResourceView* pShaderResourceView = (pDisplayParams->MultisampleQualityLevel < 2) ? pRender->m_SRView1 : pRender->m_SRView2;
		pRender->m_pFXAA->Render(pShaderResourceView);
	}

	s_FrameTimer.Update();
	m_fFrameTime = s_FrameTimer.GetDelta();

	//TOSHI_INFO("Update time of ARenderer was: {}", m_fUpdateTime);

	s_FrameTimer.Reset();

	pRender->EndScene();
}

TBOOL ARenderer::CreateInterface()
{
	// 005ed3b0
	TOrderTable::CreateStaticData(4500, 2500);
	return CreateTRender();
}

TBOOL ARenderer::CreateTRender()
{
	// 005f0780
	auto renderer = new TRenderDX11();
	renderer->Create();

	TRender::DisplayParams displayParams;

	renderer->BuildAdapterDatabase();
	auto adapterList = renderer->GetAdapterList();
	
	auto adapter = adapterList->Head()->As<TD3DAdapter>();
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
				T2String8::Format(formattedString, "The specified width and heith must be between 1280x720 and the current desktop resolution (%dx%d)", mode->GetWidth(), mode->GetHeight());
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
	TPrimShader::CreateSingleton();

	if (error == TTRB::ERROR_OK)
	{
		TAssetInit::InitAssets(s_BootAssetsTRB, true, false);
	}

	// Create SysShader
	TSysShaderHAL::OrderTablePriorities priorities = {
		100,
		5220,
		5222,
		5225,
		5215,
		500,
		5219
	};

	auto pSysShader = TSysShaderHAL::CreateSingleton<TSysShaderHAL>(&priorities);
	pSysShader->Create();

	CreateMainViewport();

	m_pViewportManager = new AXYZViewportManager();

	float width, height;

	T2GUI::GetSingletonWeak()->GetRootElement()->GetDimensions(width, height);

	m_pRectangle = new T2GUIRectangle();
	
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

	m_BackgroundColor = TCOLOR(r, g, b);
}

void ARenderer::RenderLoadingScreen(float fDeltaTime)
{
	TIMPLEMENT();
}

void ARenderer::RenderGUI(TBOOL allowBackgroundClear)
{
	TIMPLEMENT();

	if (m_bRenderGUI)
	{
		m_pViewport->AllowBackgroundClear(allowBackgroundClear);
		m_pViewport->Begin();
		T2GUI::GetSingletonWeak()->Render();
		m_pViewport->End();
	}
}

void ARenderer::RenderMainScene(float deltaTime, TViewport* pViewport, TCameraObject* pCameraObject, ACamera* pCamera, t_MainScene mainSceneCb, TBOOL bAllowBackgroundClear)
{
	TIMPLEMENT();
	auto pRender = TRenderDX11::Interface();
	
	pViewport->AllowDepthClear(true);

	auto pOriginalContext = pRender->GetCurrentRenderContext();
	pRender->SetCurrentRenderContext(pViewport->GetRenderContext());

	pViewport->AllowBackgroundClear(bAllowBackgroundClear);
	pViewport->Begin();

	auto pTestModel = ATestModel::GetSingletonWeak();
	
	if (pTestModel)
	{
		pTestModel->Render(deltaTime);
	}

	// ...
	mainSceneCb(deltaTime, pCameraObject);
	pViewport->End();
	pRender->FlushShaders();

	pRender->ResolveSubresource();
	pRender->m_pGlow->Render(pRender->m_SRView2, TNULL);
	// ...

	pRender->FlushShaders();
	pRender->SetCurrentRenderContext(pOriginalContext);
}

void ARenderer::CreateMainViewport()
{
	auto renderer = TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

	m_pViewport = new TViewport();
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

void ARenderer::GetAppCamera()
{
	TASSERT(ACameraManager::IsSingletonCreated());

	auto pCamMngr = ACameraManager::GetSingletonWeak();

	for (int i = AXYZViewportManager::VIEWPORT_Count - 1; i >= 0; i--)
	{
		if (m_pViewportManager->IsViewportCameraUsed(i))
		{
			auto pCamera = m_pViewportManager->GetViewportCamera(i);

			if (pCamera == TNULL)
				pCamera = pCamMngr->GetCurrentCamera();

			if (pCamera == TNULL)
				continue;

			auto cameraMatrix = pCamera->GetCameraMatrix();
			TMatrix44 cameraFinalMatrix = cameraMatrix.m_mMatrix;
			TTODO("Apply shake matrix");
			
			auto pCameraObject = m_pViewportManager->GetViewportCameraObject(i);
			pCameraObject->SetProjectionCentreX(cameraMatrix.m_fProjectionCentreX);
			pCameraObject->SetProjectionCentreY(cameraMatrix.m_fProjectionCentreY);
			pCameraObject->SetFOV(cameraMatrix.m_fFOV);
			pCameraObject->SetNear(cameraMatrix.m_fNear);
			pCameraObject->SetFar(cameraMatrix.m_fFar);
			
			auto& pCameraTransform = pCameraObject->GetTransformObject();
			pCameraTransform.SetMatrix(cameraFinalMatrix);
		}
	}
}

void ARenderer::InitialiseViewPort()
{
	uint8_t r = TCOLOR_GET_RED(m_BackgroundColor);
	uint8_t g = TCOLOR_GET_GREEN(m_BackgroundColor);
	uint8_t b = TCOLOR_GET_BLUE(m_BackgroundColor);

	for (int i = AXYZViewportManager::VIEWPORT_Count - 1; i >= 0; i--)
	{
		if (m_pViewportManager->IsViewportCameraUsed(i))
		{
			m_pViewportManager->GetViewport(i)->SetBackgroundColor(r, g, b, 0xFF);
		}
	}
}

void ARenderer::CreateTRenderResources()
{
	// Since this is deprecated and factory resource classes don't even exist, TSysShader is not used too
	TASSERT(TFALSE, "Deprecated in de blob");
}