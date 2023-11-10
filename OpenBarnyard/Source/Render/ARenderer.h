#pragma once
#include TOSHI_MULTIPLATFORM(ADisplayModes)

#include <Toshi/Core/TTask.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/Render/TCameraObject.h>
#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/Render/TRenderAdapter.h>

TOBJECT(ARenderer, Toshi::TTask, TTRUE),
public Toshi::TSingleton<ARenderer>
{
public:
	ARenderer();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(float a_fDeltaTime) override;

	TBOOL CreateTRender();
	TBOOL CreateTRenderResources();

	Toshi::TRenderAdapter::Mode::Device* FindSuitableDevice(Toshi::TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder);

	Toshi::TGenericEmitter& GetRenderGUIEmitter() { return m_RenderGUIEmitter; }
	const Toshi::TGenericEmitter& GetRenderGUIEmitter() const { return m_RenderGUIEmitter; }

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
	Toshi::TGenericEmitter m_AnimationUpdateStartEmitter;
	Toshi::TGenericEmitter m_AnimationUpdateEndEmitter;
	Toshi::TEmitter<ARenderer, TINT> m_RenderGUIEmitter;
	ADisplayModes_Win m_DisplayModes;
};
