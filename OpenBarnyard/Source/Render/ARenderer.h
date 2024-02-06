#pragma once

#ifdef TOSHI_SKU_WINDOWS
#include "Platform/Windows/ADisplayModes_Win.h"
#endif // TOSHI_SKU_WINDOWS

#include <Toshi/Core/TTask.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/Render/TCameraObject.h>
#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/Render/TRenderAdapter.h>

class ARenderer :
	public Toshi::TTask,
	public Toshi::TSingleton<ARenderer>
{
public:
	TDECLARE_CLASS(Toshi::TTask);

public:
	ARenderer();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(float a_fDeltaTime) override;

	TBOOL CreateTRender();
	TBOOL CreateTRenderResources();

	void ForceUpdate30FPS();

	Toshi::TRenderAdapter::Mode::Device* FindSuitableDevice(Toshi::TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder);

	Toshi::TGenericEmitter& GetRenderGUIEmitter() { return m_RenderGUIEmitter; }
	const Toshi::TGenericEmitter& GetRenderGUIEmitter() const { return m_RenderGUIEmitter; }

private:
	void CreateMainViewport();
	void RenderGUI();

private:
	Toshi::TViewport* m_pViewport;
	Toshi::TCameraObject* m_pCameraObject;
	Toshi::TViewport* m_pHALViewport1;
	Toshi::TViewport* m_pHALViewport2;
	TFLOAT m_fFar;
	TFLOAT m_fNear;
	TUINT32 m_eScreenCaptureStatus;
	Toshi::TGenericEmitter m_AnimationUpdateStartEmitter;
	Toshi::TGenericEmitter m_AnimationUpdateEndEmitter;
	Toshi::TEmitter<ARenderer, TINT> m_RenderGUIEmitter;
	ADisplayModes_Win m_DisplayModes;
};
