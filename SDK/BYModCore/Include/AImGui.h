#pragma once
#include "imgui.h"
#include "imgui_internal.h"

#include <Render/TRenderInterface.h>
#include <Input/TInputInterface.h>

class MODCORE_API AImGUI :
	public Toshi::TSingleton<AImGUI>
{
public:
	AImGUI();

	void Toggle();

	void BeginScene();
	void EndScene();
	void Render();
	void RenderOverlay();
	void OnD3DDeviceLost();
	void OnD3DDeviceFound();

	void Enable( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }
	TBOOL IsEnabled() { return m_bEnabled; }

	void LockInput();
	void UnlockInput();

	TBOOL OnInputEvent( Toshi::TInputInterface::InputEvent* a_pInputEvent );

private:
	Toshi::TRenderInterface::DISPLAYPARAMS m_DisplayParams;
	TBOOL								   m_bEnabled = TFALSE;

	TINT m_iNumInputLocks = 0;
};

extern "C" MODCORE_API AImGUI* g_pImGui;
