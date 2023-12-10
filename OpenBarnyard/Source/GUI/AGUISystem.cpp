#include "pch.h"
#include "AGUISystem.h"
#include "AGUI2TextureSectionManager.h"
#include "Render/ARenderer.h"
#include "Memory/AMemory.h"
#include "Terrain/ATerrain.h"

TPSTRING8_DECLARE(Start);
TPSTRING8_DECLARE(ParkManager);

AGUISystem::AGUISystem()
{
	m_Unk1 = 0;
	m_Unk2 = 0;
	m_Unk3 = 0;
	m_pStrings.Create(2);
	m_pStrings[0] = TPS8(ParkManager);
	m_pStrings[1] = TPS8(Start);
}

TBOOL AGUISystem::OnCreate()
{
	m_RenderGUIListener.Connect(
		ARenderer::GetSingleton()->GetRenderGUIEmitter(),
		this,
		OnGUIRender,
		0xffff8000
	);

	auto pDisplayParams = Toshi::TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	for (TUINT i = 0; i < NUM_RENDER_OBJECTS; i++)
	{
		auto pViewport = new Toshi::TViewport(TTRUE);
		pViewport->SetX(0.0f);
		pViewport->SetY(0.0f);
		pViewport->SetWidth(TFLOAT(pDisplayParams->uiWidth));
		pViewport->SetHeight(TFLOAT(pDisplayParams->uiHeight));
		pViewport->SetMinZ(0.0f);
		pViewport->SetMaxZ(1.0f);
		pViewport->SetBackgroundColor(0, 0, 0, 0);
		pViewport->AllowBackgroundClear(TFALSE);
		pViewport->AllowDepthClear(TTRUE);
		pViewport->SetMemoryAllocatorBlock(AMemory::GetMemBlock(AMemory::POOL_Viewport));

		m_pRenderObjects[i] = new PGUITRRenderObject(i);
		m_pRenderObjects[i]->SetViewport(pViewport);
	}

	return TTRUE;
}

void AGUISystem::OnDestroy()
{
	for (TUINT i = 0; i < NUM_RENDER_OBJECTS; i++)
	{
		delete m_pRenderObjects[i];
	}

	m_SomeListener.Disconnect();
}

TBOOL AGUISystem::OnGUIRender(AGUISystem* a_pGUISystem, ARenderer* a_pRenderer, TINT* a_pUnknown)
{
	if (*a_pUnknown == 0)
	{
		a_pGUISystem->m_MatLibPicture.Render();
	}

	return TTRUE;
}

void AGUISystem::AllowBackgroundClear(TBOOL a_bAllow)
{
	if (ms_pSingleton->m_pRenderObjects[0] != TNULL)
	{
		auto pViewport = ms_pSingleton->m_pRenderObjects[0]->GetViewport();

		if (pViewport)
		{
			pViewport->AllowBackgroundClear(a_bAllow);
		}
	}
}

void AGUISystem::SetPicture(const Toshi::TPString8& a_rName)
{
	TBOOL bShowPicture = TFALSE;

	if (!a_rName.IsEmpty() && a_rName.GetString8().Length() != 0)
	{
		bShowPicture = (a_rName != m_MatLibPicture.GetName());
	}

	if (bShowPicture)
	{
		if (ATerrain::IsSingletonCreated())
		{
			ATerrain::GetSingleton()->WaitUntilLoaded();
		}

		m_MatLibPicture.Create(a_rName);
		m_MatLibPicture.Flush();
		m_MatLibPicture.Cache();
		m_MatLibPicture.SetVisible();
	}
	else if (a_rName.IsEmpty() || a_rName.GetString8().Length() == 0)
	{
		m_MatLibPicture.Flush();
		m_MatLibPicture.Create(a_rName);
		m_MatLibPicture.SetHidden();
	}
}
