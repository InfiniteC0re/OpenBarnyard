#include "pch.h"
#include "ARenderer.h"
#include "AppBoot.h"
#include "Movie/AMoviePlayer.h"

#include <Platform/Windows/DX11/TRender_DX11.h>
#include <Platform/Windows/DX11/TRenderContext_DX11.h>
#include <Platform/Windows/DX11/TPrimShader_DX11.h>
#include <Toshi/Render/TAssetInit.h>

Toshi::TTRB ARenderer::s_BootAssetsTRB = Toshi::TTRB();

static void MainScene(float deltaTime, void* pCameraObject)
{
	// Test code
	auto pRender = Toshi::TRenderDX11::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();
	auto pRenderContext = (Toshi::TRenderContextDX11*)pRender->GetCurrentRenderContext();
	pRenderContext->SetCameraMode(Toshi::TRenderContext::CameraMode_Orthographic);

	static bool s_IsMatrixSet = false;
	static Toshi::TMatrix44 s_IdentityMatrix;

	if (!s_IsMatrixSet)
	{
		s_IdentityMatrix.Identity();
		s_IsMatrixSet = true;
	}

	pRenderContext->SetModelViewMatrix(s_IdentityMatrix);
	pRenderContext->SetWorldViewMatrix(s_IdentityMatrix);

	Toshi::TRenderContext::PROJECTIONPARAMS projParams;
	projParams.m_Proj.x = 1.0f;
	projParams.m_Proj.y = -1.0f;
	projParams.m_Centre.x = pDisplayParams->Width * 0.5f;
	projParams.m_Centre.y = pDisplayParams->Height * 0.5f;
	projParams.m_fNearClip = 0.0f;
	projParams.m_fFarClip = 1.0f;

	pRenderContext->SetProjectionParams(projParams);
	pRenderContext->Update();

	auto pPrimShader = Toshi::TPrimShader::GetSingletonWeak();
	pPrimShader->StartRendering(Toshi::TPrimShader::PrimType_LineList);
	pPrimShader->GetCurrentVertex()->UV.x = 0;
	pPrimShader->GetCurrentVertex()->UV.y = 0;
	pPrimShader->GetCurrentVertex()->Position.x = 0.0f;
	pPrimShader->GetCurrentVertex()->Position.y = 0.0f;
	pPrimShader->GetCurrentVertex()->Position.z = 0.0f;
	pPrimShader->GetCurrentVertex()->Color = 0xFFFFFFFF;
	pPrimShader->AddVert();
	pPrimShader->GetCurrentVertex()->UV.x = 0;
	pPrimShader->GetCurrentVertex()->UV.y = 0;
	pPrimShader->GetCurrentVertex()->Position.x = 1.0f;
	pPrimShader->GetCurrentVertex()->Position.y = 0.0f;
	pPrimShader->GetCurrentVertex()->Position.z = 0.0f;
	pPrimShader->GetCurrentVertex()->Color = 0xFFFFFFFF;
	pPrimShader->AddVert();
	pPrimShader->StopRendering();
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

	ID3D11ShaderResourceView* pShaderResourceView = (pDisplayParams->MultisampleQualityLevel < 2) ? pRender->m_SRView1 : pRender->m_SRView2;
	pRender->m_pFXAA->Render(pShaderResourceView);

	pRender->EndScene();
}

bool ARenderer::CreateInterface()
{
	// 005ed3b0
	TTODO("Toshi::TOrderTable::CreateStaticData");
	return CreateTRender();
}

bool ARenderer::CreateTRender()
{
	// 005f0780
	auto renderer = new Toshi::TRenderDX11();
	renderer->Create();

	Toshi::TRender::DisplayParams displayParams;
	displayParams.Unk3 = 32;
	displayParams.Unk4 = 3;
	displayParams.Unk5 = true;
	displayParams.IsFullscreen = false;
	displayParams.MultisampleQualityLevel = 1;
	displayParams.Width = 800;
	displayParams.Height = 600;

	renderer->CreateDisplay(&displayParams);

	TTODO("The whole function");

	return true;
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

	CreateMainViewport();
}

void ARenderer::SetBackgroundColour(uint32_t r, uint32_t g, uint32_t b)
{
	if (m_pViewport != TNULL)
	{
		m_pViewport->SetBackgroundColor(r & 0xFF, g & 0xFF, b & 0xFF, 255);
	}

	m_BackgroundColor = ((r & 0xff) << 8 | g & 0xff) << 8 | b & 0xff;
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
	mainSceneCb(deltaTime, pCameraObject);
	pViewport->End();
	// ...

	pRender->FUN_00691190();
	pRender->SetCurrentRenderContext(pOriginalContext);
}

void ARenderer::CreateMainViewport()
{
	TIMPLEMENT();
	auto pDisplayParams = Toshi::TRenderDX11::Interface()->GetCurrentDisplayParams();

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
	m_pViewport->EnableDefaultBeginRender(true);
	m_pViewport->SetBackgroundColor(0, 0, 0, 255);
}
