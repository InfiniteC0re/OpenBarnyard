#include "pch.h"
#include "AXYZViewportManager.h"
#include <Memory/AMemory.h>
#include <Platform/Windows/DX11/TRender_DX11.h>

AXYZViewportManager::AXYZViewportManager()
{
	TIMPLEMENT();
	auto renderer = Toshi::TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

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

void AXYZViewportManager::SetMainFullScreen(bool fullscreen)
{
	TIMPLEMENT();
}
