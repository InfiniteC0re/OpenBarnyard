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

void ARenderer::Update(float deltaTime)
{
	TIMPLEMENT();
	Toshi::TRenderDX11* pRender = static_cast<Toshi::TRenderDX11*>(Toshi::TRender::GetSingletonWeak());

	pRender->Update(deltaTime);
	pRender->BeginScene();

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
