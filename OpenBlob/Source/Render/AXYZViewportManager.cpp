#include "pch.h"
#include "AXYZViewportManager.h"
#include <Memory/AMemory.h>
#include <Platform/Windows/DX11/TRender_DX11.h>

AXYZViewportManager::AXYZViewportManager()
{
	TIMPLEMENT();
	auto renderer = Toshi::TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

	float x, y, w, h;
	GetUseableArea(x, y, w, h);


}

Toshi::TViewport* AXYZViewportManager::CreateViewport(float x, float y, float w, float h)
{
	auto pViewport = new Toshi::TViewport();
	pViewport->SetMemoryAllocatorBlock(AMemory::ms_apMemoryBlocks[AMemory::POOL_FrequentAllocations]);
	pViewport->SetX(x);
	pViewport->SetY(y);
	pViewport->SetWidth(w);
	pViewport->SetHeight(h);
	pViewport->SetMinZ(0.2f);
	pViewport->SetMaxZ(1.0f);
	pViewport->AllowBackgroundClear(true);
	pViewport->AllowDepthClear(true);
	pViewport->Enable(true);
	pViewport->EnableDefaultBeginRender(true);
	pViewport->SetBackgroundColor(64, 64, 64, 255);

	return pViewport;
}

void AXYZViewportManager::GetUseableArea(float& x, float& y, float& w, float& h)
{
	TIMPLEMENT();
	auto renderer = Toshi::TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

	//x = pDisplayParams->Width / 3.0f;
	//y = pDisplayParams->
	w = pDisplayParams->Width * 0.5f;
	h = pDisplayParams->Height * 0.5f;
}

void AXYZViewportManager::SetMainFullScreen(bool fullscreen)
{
	TIMPLEMENT();
}
