#include "pch.h"
#include "ARenderer.h"

#include TOSHI_MULTIRENDER(TRenderInterface)

TOSHI_NAMESPACE_USING

TBOOL ARenderer::CreateTRender()
{
	TIMPLEMENT();

	auto pRendererClass = TClass::Find("TRenderD3DInterface");
	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, pRendererClass->CreateObject());

	TBOOL bCreated = pRenderer->Create("Barnyard");

	if (bCreated)
	{
		TRenderInterface::DISPLAYPARAMS displayParams;
		displayParams.uiWidth = 800;
		displayParams.uiHeight = 600;
		displayParams.bWindowed = TTRUE;
		displayParams.uiColourDepth = 32;
		displayParams.eDepthStencilFormat = 0;

		pRenderer->CreateDisplay(displayParams);
		pRenderer->DumpStats();
	}

	return bCreated;
}

TBOOL ARenderer::OnCreate()
{
	TIMPLEMENT();

	TOrderTable::CreateStaticData(2000, 4000);
	return CreateTRender();
}
