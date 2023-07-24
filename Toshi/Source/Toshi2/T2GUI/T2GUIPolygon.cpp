#include "ToshiPCH.h"
#include "T2GUIPolygon.h"

namespace Toshi
{
	void T2GUIPolygon::Render()
	{
		auto renderer = T2GUI::GetSingleton()->GetRenderer();
		renderer->SetMaterial(m_pMaterial);
		renderer->SetColour(m_Color);
		renderer->RenderTriStrip(m_pVertices, m_pUV, m_iNumVerts, 1.0f, 1.0f);
		T2GUIElement::Render();
	}

	void T2GUIPolygon::GetDimensions(float& width, float& height)
	{
		if (!m_bCalculatedDimensions)
		{
			float minX = TMath::MAXFLOAT;
			float minY = TMath::MAXFLOAT;
			float maxX = TMath::MINFLOAT;
			float maxY = TMath::MINFLOAT;

			for (size_t i = 0; i < m_iNumVerts; i++)
			{
				minX = TMath::Min(minX, m_pVertices[i].x);
				minY = TMath::Min(minY, m_pVertices[i].y);
				maxX = TMath::Max(maxX, m_pVertices[i].x);
				maxY = TMath::Max(maxY, m_pVertices[i].y);
			}

			m_bCalculatedDimensions = TTRUE;
			m_Width = PackFloat(maxX - minX);
			m_Height = PackFloat(maxY - minY);
		}

		width = UnpackFloat(m_Width);
		height = UnpackFloat(m_Height);
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