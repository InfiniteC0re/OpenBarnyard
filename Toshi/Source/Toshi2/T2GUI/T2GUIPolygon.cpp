#include "ToshiPCH.h"
#include "T2GUIPolygon.h"

namespace Toshi
{
	void T2GUIPolygon::Render()
	{
		auto renderer = T2GUI::GetSingletonWeak()->GetRenderer();
		renderer->SetMaterial(m_pMaterial);
		renderer->SetColour(m_Color);
		renderer->RenderTriStrip(m_pVertices, m_pUV, m_iNumVerts, 1.0f, 1.0f);
		T2GUIElement::Render();
	}

	void T2GUIPolygon::GetDimensions(float& width, float& height)
	{
		if (!m_bSomethingDimensions)
		{
			TIMPLEMENT();
		}
		T2GUIElement::GetDimensions(width, height);
	}

	void T2GUIPolygon::SetDimensions(float width, float height)
	{
		TASSERT(TFALSE, "T2GUIPolygon::SetDimensions(): Can\'t set T2GUIPolygon dimensions directly!");
	}

	void T2GUIPolygon::Create(int numpoints)
	{
		TASSERT(numpoints > 2);
		m_iNumVerts = numpoints;
		m_pVertices = new TVector2[numpoints];
		m_pUV = new TVector2[numpoints];
	}
}