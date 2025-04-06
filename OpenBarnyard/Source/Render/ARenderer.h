#pragma once
#include "Cameras/ACamera.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/ADisplayModes_Win.h"
#endif // TOSHI_SKU_WINDOWS

#include <Toshi/TTask.h>
#include <Render/TViewport.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderAdapter.h>

class ARenderer : public Toshi::TTask
    , public Toshi::TSingleton<ARenderer>
{
public:
	TDECLARE_CLASS( ARenderer, Toshi::TTask );

public:
	ARenderer();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	TBOOL CreateTRender();
	TBOOL CreateTRenderResources();

	void UpdateMainCamera( const Toshi::TMatrix44& a_rTransformMatrix, const ACamera* a_pCamera );

	void ForceUpdate30FPS();

	Toshi::TRenderAdapter::Mode::Device* FindSuitableDevice( Toshi::TRenderInterface::DISPLAYPARAMS& a_rDisplayParams, bool a_bReverseOrder );

	Toshi::TViewport* GetMainViewport() const { return m_pViewport; }

	Toshi::TGenericEmitter&       GetRenderGUIEmitter() { return m_RenderGUIEmitter; }
	const Toshi::TGenericEmitter& GetRenderGUIEmitter() const { return m_RenderGUIEmitter; }

private:
	void CreateMainViewport();
	void RenderGUI();
	void RenderMainScene( TFLOAT a_fDeltaTime );

private:
	Toshi::TViewport*                  m_pViewport;
	Toshi::TCameraObject*              m_pCameraObject;
	Toshi::TViewport*                  m_pHALViewport1;
	Toshi::TViewport*                  m_pHALViewport2;
	TFLOAT                             m_fFar;
	TFLOAT                             m_fNear;
	TUINT32                            m_eScreenCaptureStatus;
	Toshi::TEmitter<ARenderer, TBOOL*> m_AnimationUpdateStartEmitter;
	Toshi::TEmitter<ARenderer, TBOOL*> m_AnimationUpdateEndEmitter;
	Toshi::TEmitter<ARenderer, TINT>   m_RenderGUIEmitter;
	TBOOL                              m_bRenderGUI;
	ADisplayModes_Win                  m_DisplayModes;
};
