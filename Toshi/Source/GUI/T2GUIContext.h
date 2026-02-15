#pragma once

TOSHI_NAMESPACE_START

class T2GUIRenderer;
class T2GUIElement;

class T2GUIContext
{
public:
	T2GUIContext();

	void Tick( TFLOAT a_fDeltaTime );
	void Render();

	T2GUIRenderer* GetRenderer() const { return m_pRenderer; }
	T2GUIElement*  GetRootElement() const { return m_pRootElement; }
	T2GUIElement*  GetDebugCanvas() const { return m_pDebugCanvas; }

	void SetRenderer( T2GUIRenderer* a_pRenderer ) { m_pRenderer = a_pRenderer; }
	void SetRootElement( T2GUIElement* a_pElement ) { m_pRootElement = a_pElement; }

private:
	T2GUIRenderer* m_pRenderer;
	T2GUIElement*  m_pRootElement;
	T2GUIElement*  m_pCreatedRootElement;
	T2GUIElement*  m_pDebugCanvas;
};

TOSHI_NAMESPACE_END
