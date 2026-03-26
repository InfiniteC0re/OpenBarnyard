#pragma once
#include "THookedSingleton.h"

#include <Render/TViewport.h>
#include <Render/TCameraObject.h>

class ADisplayModes_Win : public Toshi::TObject
{
public:
	struct Mode
	{
		TUINT32 m_uiWidth;
		TUINT32 m_uiHeight;
		TUINT32 m_uiColourDepth;
	};

public:
	Toshi::T2Vector<Mode, 1024> m_Modes;
};

class ARenderer
	: public Toshi::TTask
	, public THookedSingleton<ARenderer, 0x0079b2ec>
{
public:
	TCHAR PAD0[ 4 ];

	Toshi::TViewport*                  m_pViewport;
	Toshi::TCameraObject*              m_pCameraObject;
	Toshi::TViewport*                  m_pHALViewport1;
	Toshi::TViewport*                  m_pHALViewport2;
	TFLOAT                             m_fFar;
	TFLOAT                             m_fNear;
	TUINT32                            m_eScreenCaptureStatus;

	TCHAR PAD1[ 24 ];

	Toshi::TEmitter<ARenderer, TBOOL*> m_AnimationUpdateStartEmitter;
	Toshi::TEmitter<ARenderer, TBOOL*> m_AnimationUpdateEndEmitter;
	Toshi::TEmitter<ARenderer, TINT>   m_RenderGUIEmitter;

	void* PAD2;
	void* PAD3;
	void* PAD4;

	TBOOL                              m_bRenderGUI;
	ADisplayModes_Win                  m_DisplayModes;
};
