#pragma once
#include "AGUI2Renderer.h"
#include "SDK_T2GUIElement.h"

class SDK_T2GUIContext
{
public:
	SDK_T2GUIContext();

	void Tick( TFLOAT a_fDeltaTime );
	void Render();

	SDK_T2GUIRenderer* GetRenderer() const { return m_pRenderer; }
	SDK_T2GUIElement*  GetRootElement() const { return m_pRootElement; }
	SDK_T2GUIElement*  GetDebugCanvas() const { return m_pDebugCanvas; }

	void SetRenderer( SDK_T2GUIRenderer* a_pRenderer ) { m_pRenderer = a_pRenderer; }
	void SetRootElement( SDK_T2GUIElement* a_pElement ) { m_pRootElement = a_pElement; }

private:
	SDK_T2GUIRenderer* m_pRenderer;
	SDK_T2GUIElement*  m_pRootElement;
	SDK_T2GUIElement*  m_pCreatedRootElement;
	SDK_T2GUIElement*  m_pDebugCanvas;
};
