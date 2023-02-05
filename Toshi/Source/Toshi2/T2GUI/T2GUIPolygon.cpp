#include "ToshiPCH.h"
#include "T2GUIPolygon.h"


void Toshi::T2GUIPolygon::Render()
{
	auto renderer = T2GUI::GetSingletonWeak()->GetRenderer();
	renderer->SetMaterial(m_pMaterial);
	renderer->SetColour(m_Color);
	renderer->RenderTriStrip(m_pUnk, m_pUnk2, m_iNumVerts, 1.0f, 1.0f);
	T2GUIElement::Render();
}

void Toshi::T2GUIPolygon::GetDimensions(float& width, float& height)
{
	if (!m_bSomethingDimensions)
	{
		TIMPLEMENT();
	}
	T2GUIElement::GetDimensions(width, height);
}

void Toshi::T2GUIPolygon::Create(int numpoints)
{
	TASSERT(numpoints > 2);
	m_iNumVerts = numpoints;
	m_pUnk = new TVector2[numpoints];
	m_pUnk2 = new TVector2[numpoints];
}
