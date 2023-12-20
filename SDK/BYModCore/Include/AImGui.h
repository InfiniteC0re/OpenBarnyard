#pragma once
#include "imgui.h"
#include "imgui_internal.h"

#include <Toshi/Render/TRenderInterface.h>

class MODCORE_API AImGUI :
	public Toshi::TSingleton<AImGUI>
{
public:
	AImGUI();

	void Toggle();

	void BeginScene();
	void EndScene();
	void Render();
	void OnD3DDeviceLost();
	void OnD3DDeviceFound();

	void Enable(TBOOL a_bEnabled)
	{
		m_bEnabled = a_bEnabled;
	}

	TBOOL IsEnabled()
	{
		return m_bEnabled;
	}

private:
	Toshi::TRenderInterface::DISPLAYPARAMS m_DisplayParams;
	TBOOL m_bEnabled = TFALSE;
};
