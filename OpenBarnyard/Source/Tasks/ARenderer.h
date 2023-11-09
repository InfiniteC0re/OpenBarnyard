#pragma once
#include "ADisplayModes_Win.h"

#include <Toshi/Core/TTask.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/Render/TCameraObject.h>
#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/Render/TRenderAdapter.h>

class ARenderer :
	public Toshi::TGenericClassDerived<ARenderer, Toshi::TTask, "ARenderer", TMAKEVERSION(1, 0), TTRUE>
{
public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(float a_fDeltaTime) override;

	TBOOL CreateTRender();
	TBOOL CreateTRenderResources();

	Toshi::TRenderAdapter::Mode::Device* FindSuitableDevice(Toshi::TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder);

private:
	void RenderGUI();

private:
	Toshi::TViewport* m_pViewport;
	Toshi::TCameraObject m_pCameraObject;
	Toshi::TViewport* m_pHALViewport1;
	Toshi::TViewport* m_pHALViewport2;
	TFLOAT m_fFar;
	TFLOAT m_fNear;
	TUINT32 m_eScreenCaptureStatus;
	ADisplayModes_Win m_DisplayModes;
};
