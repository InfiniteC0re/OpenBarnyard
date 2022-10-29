#include "pch.h"
#include "ARenderer.h"

#include <Platform/Windows/DX11/TRender_DX11.h>

ARenderer::ARenderer()
{
	// 005ed0e0
}

ARenderer::~ARenderer()
{
	// 005f0140
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
	renderer->Update();

	TTODO("The whole function");

	return true;
}
