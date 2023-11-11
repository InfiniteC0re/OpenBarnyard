#include "pch.h"
#include "AppBoot.h"
#include "Assets/AMaterialLibraryManager.h"
#include "AGUI/AGUISystem.h"
#include "AGUI/AGUI2.h"
#include "AGUI/AGUI2TextureSectionManager.h"
#include "Render/ARenderer.h"
#include "Input/AInputHandler.h"

#include TOSHI_MULTIRENDER(TTextureFactoryHAL)
#include TOSHI_MULTIRENDER(TRenderInterface)

TOSHI_NAMESPACE_USING

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

	TResource* pTextureFactory = pRenderer->CreateResource(
		TClass::Find("TTextureFactoryHAL"),
		"TextureFactory",
		TNULL
	);

	pRenderer->SetResourceExplicit(pTextureFactory, TRenderInterface::SYSRESOURCE_TEXTUREFACTORY);
	pTextureFactory->Create();

	return TTRUE;
}

Toshi::TRenderAdapter::Mode::Device* ARenderer::FindSuitableDevice(TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder)
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
	
	AGUI2::GetContext()->GetRenderer()->BeginScene();
	AGUI2::GetContext()->GetRenderer()->SetColour(TCOLOR(255, 255, 255));

	AGUI2::GetContext()->GetRenderer()->SetMaterial(AGUI2TextureSectionManager::GetTextureSection("Panel_01")->GetMaterial());
	AGUI2::GetContext()->GetRenderer()->RenderRectangle({ 80.0f, 25.0f }, { -80.0f, -25.0f }, { 1.0f, 1.0f }, { 0, 0 });

	auto pSection = AGUI2TextureSectionManager::GetTextureSection("Putt_Ball");
	AGUI2::GetContext()->GetRenderer()->SetMaterial(pSection->GetMaterial());
	AGUI2::GetContext()->GetRenderer()->RenderRectangle({ 10.0f, 10.0f }, { -10.0f, -10.0f }, { pSection->GetUVPoint2().x, pSection->GetUVPoint2().y }, { pSection->GetUVPoint1().x, pSection->GetUVPoint1().y });

	m_RenderGUIEmitter.Throw(0);
	AGUI2::GetContext()->GetRenderer()->EndScene();
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

	if (!TSystemManager::GetSingleton()->IsPaused())
	{
		pRenderer->BeginScene();

		//auto pInputHandler = AInputHandler::GetSingleton();
		//auto pMouseDevice = pInputHandler->GetMouseDevice();

		//if (pMouseDevice->GetAxisInt(Toshi::TInputDeviceMouse::AXIS_CURSOR, 0) != 0 ||
		//	pMouseDevice->GetAxisInt(Toshi::TInputDeviceMouse::AXIS_CURSOR, 1) != 0)
		//{
		//	TOSHI_INFO(
		//		"Mouse position: X = {}; Y = {}",
		//		pMouseDevice->GetAxisInt(Toshi::TInputDeviceMouse::AXIS_CURSOR, 0),
		//		pMouseDevice->GetAxisInt(Toshi::TInputDeviceMouse::AXIS_CURSOR, 1)
		//	);
		//}

		RenderGUI();

		pRenderer->EndScene();
	}
}
