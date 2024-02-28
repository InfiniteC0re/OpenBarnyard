#include "pch.h"
#include "AppBoot.h"
#include "Assets/AMaterialLibraryManager.h"
#include "GUI/AGUISystem.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "Assets/AModelLoader.h"
#include "Render/ARenderer.h"
#include "Input/AInputHandler.h"
#include "Movie/AMoviePlayer.h"

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/DX8/TVertexFactoryResource_DX8.h"
#include "Platform/DX8/TIndexFactoryResource_DX8.h"
#include "Platform/DX8/TTextureFactoryHAL_DX8.h"
#include "Platform/DX8/TRenderInterface_DX8.h"
#include "Platform/DX8/TViewport_DX8.h"

#include "Platform/DX8/ASysShader/ASysShader_DX8.h"
#include "Platform/DX8/AWorldShader/AWorldShader_DX8.h"
#endif // TOSHI_SKU_WINDOWS

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ARenderer);

ARenderer::ARenderer() :
	m_RenderGUIEmitter(this)
{

}

TBOOL ARenderer::CreateTRender()
{
	auto pRendererClass = TClass::Find("TRenderD3DInterface");
	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, pRendererClass->CreateObject());

	TFIXME("Call FUN_006c8010(DAT_007cf02c,&local_38) but not sure if it has any use");

	pRenderer->SetAutoCreateSystemResources(TFALSE);
	pRenderer->Create("Barnyard");

	m_DisplayModes.Initialise();

	CreateTRenderResources();

	TINT iWidth = 800;
	TINT iHeight = 600;
	TINT iColourDepth = 32;
	TBOOL bWindowed = TTRUE;

	if (AOptions::GetSingleton())
	{
		iWidth = AOptions::GetSingleton()->GetWidth();
		iHeight = AOptions::GetSingleton()->GetHeight();
		bWindowed = AOptions::GetSingleton()->IsWindowed();
		pRenderer->SetGamma(AOptions::GetSingleton()->GetGamma());
	}

	if (!m_DisplayModes.IsModeExists(iWidth, iHeight, iColourDepth))
	{
		// Incompatible display mode
		iWidth = 800;
		iHeight = 600;
		iColourDepth = 32;
	}

	TRenderInterface::DISPLAYPARAMS displayParams;
	displayParams.uiWidth = iWidth;
	displayParams.uiHeight = iHeight;
	displayParams.bWindowed = bWindowed;
	displayParams.uiColourDepth = 32;
	displayParams.eDepthStencilFormat = 0;

	auto pDevice = FindSuitableDevice(displayParams, TTRUE);

	if (!pDevice)
	{
		if (displayParams.uiColourDepth == 32)
		{
			displayParams.uiColourDepth = 16;
			pDevice = FindSuitableDevice(displayParams, TTRUE);

			if (pDevice == 0)
			{
				pRenderer->OnInitializationFailureDisplay();
				return TFALSE;
			}
		}
		else
		{
			pRenderer->OnInitializationFailureDisplay();
			return TFALSE;
		}
	}
	
	if (pRenderer->CreateDisplay(displayParams))
	{
		TTODO("Log info about choosen device");
		return TTRUE;
	}

	return TFALSE;
}

TBOOL ARenderer::CreateTRenderResources()
{
	TIMPLEMENT();

	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());

	{
		auto pResource = TSTATICCAST(
			TVertexFactoryResource*,
			pRenderer->CreateResource(
				TClass::Find("TVertexFactoryResource"),
				"VFSYSVNDUV1",
				TNULL
			)
		);

		pRenderer->SetResourceExplicit(pResource, SYSRESOURCE_VFSYSVNDUV1);

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams = 1;
		vertexFormat.m_aStreamFormats[0].m_uiVertexSize = 24;
		vertexFormat.m_aStreamFormats[0].m_Unk = 0;
		pResource->Create(&vertexFormat, 54050, 0);
	}

	{
		auto pResource = TSTATICCAST(
			TVertexFactoryResource*,
			pRenderer->CreateResource(
				TClass::Find("TVertexFactoryResource"),
				"VFSKIN",
				TNULL
			)
		);

		pRenderer->SetResourceExplicit(pResource, SYSRESOURCE_VFSKIN);

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams = 1;
		vertexFormat.m_aStreamFormats[0].m_uiVertexSize = 40;
		vertexFormat.m_aStreamFormats[0].m_Unk = 0;
		pResource->Create(&vertexFormat, 54050, 0);
	}

	{
		auto pResource = TSTATICCAST(
			TVertexFactoryResource*,
			pRenderer->CreateResource(
				TClass::Find("TVertexFactoryResource"),
				"VFWORLD",
				TNULL
			)
		);

		pRenderer->SetResourceExplicit(pResource, SYSRESOURCE_VFWORLD);

		TVertexFactoryFormat vertexFormat;
		vertexFormat.m_uiNumStreams = 1;
		vertexFormat.m_aStreamFormats[0].m_uiVertexSize = 44;
		vertexFormat.m_aStreamFormats[0].m_Unk = 0;
		pResource->Create(&vertexFormat, 54050, 0);
	}

	{
		auto pResource = TSTATICCAST(
			TIndexFactoryResource*,
			pRenderer->CreateResource(
				TClass::Find("TIndexFactoryResource"),
				"IFSYS",
				TNULL
			)
		);

		pRenderer->SetResourceExplicit(pResource, SYSRESOURCE_IFSYS);

		TIndexFactoryFormat indexFormat;
		indexFormat.m_uiUnk = 2;
		pResource->Create(&indexFormat, 64700, 0);
	}

	{
		auto pResource = pRenderer->CreateResource(
			TClass::Find("TTextureFactoryHAL"),
			"TextureFactory",
			TNULL
		);

		pRenderer->SetResourceExplicit(pResource, SYSRESOURCE_TEXTUREFACTORY);
		pResource->Create();
	}

	return TTRUE;
}

TRenderAdapter::Mode::Device* ARenderer::FindSuitableDevice(TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder)
{
	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	TRenderAdapter::Mode::Device* pDevice = TNULL;

	if (a_bReverseOrder)
	{
		a_rDisplayParams.eDepthStencilFormat = 4;
		pDevice = pRenderer->FindDevice(a_rDisplayParams);

		if (!pDevice)
		{
			a_rDisplayParams.eDepthStencilFormat = 2;
			pDevice = pRenderer->FindDevice(a_rDisplayParams);

			if (!pDevice)
			{
				a_rDisplayParams.eDepthStencilFormat = 3;
				pDevice = pRenderer->FindDevice(a_rDisplayParams);

				if (!pDevice)
				{
					a_rDisplayParams.eDepthStencilFormat = 0;
					pDevice = pRenderer->FindDevice(a_rDisplayParams);

					if (!pDevice)
					{
						a_rDisplayParams.eDepthStencilFormat = 1;
						pDevice = pRenderer->FindDevice(a_rDisplayParams);
					}
				}
			}
		}
	}
	else
	{
		pDevice = pRenderer->FindDevice(a_rDisplayParams);
		a_rDisplayParams.eDepthStencilFormat = 0;

		if (!pDevice)
		{
			a_rDisplayParams.eDepthStencilFormat = 1;
			pDevice = pRenderer->FindDevice(a_rDisplayParams);

			if (!pRenderer->FindDevice(a_rDisplayParams))
			{
				a_rDisplayParams.eDepthStencilFormat = 2;
				pDevice = pRenderer->FindDevice(a_rDisplayParams);

				if (!pRenderer->FindDevice(a_rDisplayParams))
				{
					a_rDisplayParams.eDepthStencilFormat = 4;
					pDevice = pRenderer->FindDevice(a_rDisplayParams);

					if (!pRenderer->FindDevice(a_rDisplayParams))
					{
						a_rDisplayParams.eDepthStencilFormat = 3;
						pDevice = pRenderer->FindDevice(a_rDisplayParams);
					}
				}
			}
		}
	}

	return pDevice;
}

void ARenderer::RenderGUI()
{
	auto pRender = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pViewport = AGUISystem::GetSingleton()->GetRenderObject(0)->GetViewport();

	auto pOldContext = pRender->SetCurrentRenderContext(pViewport->GetRenderContext());;

	TTODO("Save and restore value of 'm_AmbientColor?' when it's figured out");

	pViewport->Begin();

	// Render UI that is not attached to context
	AGUI2::GetContext()->GetRenderer()->BeginScene();
	m_RenderGUIEmitter.Throw(0);
	AGUI2::GetContext()->GetRenderer()->EndScene();

	// Render UI that is attached to the context
	AGUI2::GetContext()->Render();
	
	pViewport->End();

	pRender->FlushShaders();
	pRender->SetCurrentRenderContext(pOldContext);
}

TBOOL ARenderer::OnCreate()
{
	TIMPLEMENT();

	TOrderTable::CreateStaticData(2000, 4000);
	TBOOL bCreatedTRender = CreateTRender();

	TASSERT(bCreatedTRender);

	if (bCreatedTRender)
	{
		(new AWorldShaderHAL())->Create();
		(new ASysShaderHAL())->Create();
		CreateMainViewport();

		AModelLoader::CreateSingleton();
		AMaterialLibraryManager::CreateSingleton();
	}

	return bCreatedTRender;
}

TBOOL ARenderer::OnUpdate(float a_fDeltaTime)
{
	TIMPLEMENT();

	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());

	if (g_oTheApp.IsDestroyed())
	{
		return TTRUE;
	}

	pRenderer->Update(a_fDeltaTime);

	if (!g_oSystemManager.IsPaused())
	{
		pRenderer->BeginScene();

		if (AMoviePlayer::IsSingletonCreated() && AMoviePlayer::GetSingleton()->IsMoviePlaying())
		{
			AMoviePlayer::GetSingleton()->Render(a_fDeltaTime);
		}

		RenderGUI();

		pRenderer->EndScene();
	}
}

void ARenderer::ForceUpdate30FPS()
{
	OnUpdate(1.0f / 30.0f);
}

void ARenderer::CreateMainViewport()
{
	auto pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();
	
	m_pViewport = new TViewport(TTRUE);
	m_pViewport->SetX(0);
	m_pViewport->SetY(0);
	m_pViewport->SetWidth(TFLOAT(pDisplayParams->uiWidth));
	m_pViewport->SetHeight(TFLOAT(pDisplayParams->uiHeight));
	m_pViewport->SetMinZ(0.2f);
	m_pViewport->SetMaxZ(1.0f);
	m_pViewport->AllowBackgroundClear(TTRUE);
	m_pViewport->AllowDepthClear(TTRUE);
	m_pViewport->SetBackgroundColor(128, 128, 128, 0);
	m_pViewport->Enable(TTRUE);
	m_pViewport->EnableDefaultBeginRender(TTRUE);
	m_pViewport->SetMemoryAllocatorBlock(AMemory::GetMemBlock(AMemory::POOL_Viewport));

	m_pHALViewport1 = new TViewportHAL(TTRUE);
	m_pHALViewport1->SetX(0);
	m_pHALViewport1->SetY(0);
	m_pHALViewport1->SetWidth(TFLOAT(pDisplayParams->uiWidth));
	m_pHALViewport1->SetHeight(TFLOAT(pDisplayParams->uiHeight));
	m_pHALViewport1->SetMinZ(0.2f);
	m_pHALViewport1->SetMaxZ(1.0f);
	m_pHALViewport1->AllowDepthClear(TFALSE);
	m_pHALViewport1->AllowBackgroundClear(TTRUE);
	m_pHALViewport1->SetBackgroundColor(0, 0, 0, 0);
	m_pHALViewport1->Enable(TTRUE);
	m_pHALViewport1->EnableDefaultBeginRender(TTRUE);
	m_pHALViewport1->SetMemoryAllocatorBlock(AMemory::GetMemBlock(AMemory::POOL_Viewport));

	m_pHALViewport2 = new TViewportHAL(TTRUE);
	m_pHALViewport2->SetX(0);
	m_pHALViewport2->SetY(0);
	m_pHALViewport2->SetWidth(TFLOAT(pDisplayParams->uiWidth));
	m_pHALViewport2->SetHeight(TFLOAT(pDisplayParams->uiHeight));
	m_pHALViewport2->SetMinZ(0.2f);
	m_pHALViewport2->SetMaxZ(1.0f);
	m_pHALViewport2->AllowBackgroundClear(TFALSE);
	m_pHALViewport2->AllowDepthClear(TFALSE);
	m_pHALViewport2->Enable(TTRUE);
	m_pHALViewport2->EnableDefaultBeginRender(TTRUE);
	m_pHALViewport2->SetMemoryAllocatorBlock(AMemory::GetMemBlock(AMemory::POOL_Viewport));

	m_pCameraObject = new TCameraObject();
}
