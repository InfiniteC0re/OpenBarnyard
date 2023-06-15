#include "pch.h"
#include "AXYZViewportManager.h"

#include <Memory/AMemory.h>
#include <Platform/Windows/DX11/TRender_DX11.h>

using namespace Toshi;

AXYZViewportManager::AXYZViewportManager()
{
	auto renderer = TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

	float x, y, w, h;
	GetUseableArea(x, y, w, h);

	float halfWidth = w * 0.5f;
	float halfHeight = h * 0.5f;
	float oneThirdWidth = w / 3.0f;

	m_Viewports[VIEWPORT_Fullscreen]            = CreateViewport(x,                     y,              w,             h         );
	m_Viewports[VIEWPORT_2X2_TOPLEFT]           = CreateViewport(x,                     y,              halfWidth,     halfHeight);
	m_Viewports[VIEWPORT_2X2_TOPRIGHT]          = CreateViewport(x + halfWidth,         y, halfWidth,   halfHeight               );
	m_Viewports[VIEWPORT_2X2_BOTTOMLEFT]        = CreateViewport(x,                     y + halfHeight, halfWidth,     halfHeight);
	m_Viewports[VIEWPORT_2X2_BOTTOMRIGHT]       = CreateViewport(x + halfWidth,         y + halfHeight, halfWidth,     halfHeight);
	m_Viewports[VIEWPORT_1P1_HORIZONTAL_TOP]    = CreateViewport(x,                     y,              w,             halfHeight);
	m_Viewports[VIEWPORT_1P1_HORIZONTAL_BOTTOM] = CreateViewport(x,                     y + halfHeight, w,             halfHeight);
	m_Viewports[VIEWPORT_1P1_VERTICAL_LEFT]     = CreateViewport(x,                     y,              halfWidth,     h         );
	m_Viewports[VIEWPORT_1P1_VERTICAL_RIGHT]    = CreateViewport(x + halfWidth,         y,              halfWidth,     h         );
	m_Viewports[VIEWPORT_1P1P1_LEFT]            = CreateViewport(x,                     y,              oneThirdWidth, h         );
	m_Viewports[VIEWPORT_1P1P1_CENTER]          = CreateViewport(x + oneThirdWidth * 1, y,              oneThirdWidth, h         );
	m_Viewports[VIEWPORT_1P1P1_RIGHT]           = CreateViewport(x + oneThirdWidth * 2, y,              oneThirdWidth, h         );

	TMatrix44 cameraMatrix;
	cameraMatrix.Identity();

	cameraMatrix.LookAtTarget(TVector4::VEC_ZERO, { 0.0f, 0.0f, 1.0f, 1.0f });
	cameraMatrix.SetTranslation({ 0.0f, 0.0f, 25.0f });

	for (size_t i = 0; i < VIEWPORT_Count; i++)
	{
		m_UsedCameras[i] = TFALSE;
		m_Cameras[i] = TNULL;

		auto pCamObject = new TCameraObject();
		pCamObject->SetProjectionCentreX(0.5f);
		pCamObject->SetProjectionCentreY(0.5f);
		pCamObject->SetFOV(sm_fDefaultFOV);
		pCamObject->SetFar(200.0f);
		pCamObject->SetNear(1.0f);

		auto& pTransformObject = pCamObject->GetTransformObject();
		pTransformObject.SetMatrix(cameraMatrix);

		m_CameraObjects[i] = pCamObject;
	}

	m_Unk = TNULL;
	m_iRenderingViewportIndex = -1;
	m_UsedCameras[VIEWPORT_Fullscreen] = TTRUE;
}

TViewport* AXYZViewportManager::CreateViewport(float x, float y, float w, float h)
{
	auto pViewport = new TViewport();

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
	auto renderer = TRenderDX11::Interface();
	auto pDisplayParams = renderer->GetCurrentDisplayParams();

	x = 0;
	y = 0;
	w = pDisplayParams->Width * 0.5f;
	h = pDisplayParams->Height * 0.5f;
}

void AXYZViewportManager::SetMainFullScreen(bool fullscreen)
{
	TIMPLEMENT();
}
