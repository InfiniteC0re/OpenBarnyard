#pragma once
#include <Toshi/Core/TTask.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/Render/TCameraObject.h>

class ARenderer :
	public Toshi::TGenericClassDerived<ARenderer, Toshi::TTask, "ARenderer", TMAKEVERSION(1, 0), TTRUE>
{
public:
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate(float a_fDeltaTime) override;

	TBOOL CreateTRender();

private:
	Toshi::TViewport* m_pViewport;
	Toshi::TCameraObject m_pCameraObject;
	Toshi::TViewport* m_pHALViewport1;
	Toshi::TViewport* m_pHALViewport2;
	TFLOAT m_fFar;
	TFLOAT m_fNear;
	TUINT32 m_eScreenCaptureStatus;
};
