#pragma once
#include "AGUI2Renderer.h"
#include "AGUI2Element.h"

class AGUI2Context
{
public:
	AGUI2Context();

	void Tick( TFLOAT a_fDeltaTime );
	void Render();

	AGUI2Renderer* GetRenderer() const { return m_pRenderer; }
	AGUI2Element*  GetRootElement() const { return m_pRootElement; }
	AGUI2Element*  GetDebugCanvas() const { return m_pDebugCanvas; }

	void SetRenderer( AGUI2Renderer* a_pRenderer ) { m_pRenderer = a_pRenderer; }
	void SetRootElement( AGUI2Element* a_pElement ) { m_pRootElement = a_pElement; }

private:
	AGUI2Renderer* m_pRenderer;
	AGUI2Element*  m_pRootElement;
	AGUI2Element*  m_pCreatedRootElement;
	AGUI2Element*  m_pDebugCanvas;
};
