#include "ToshiPCH.h"
#include "T2GUIRectangle.h"
#include "T2GUI.h"

namespace Toshi
{
	T2GUIRectangle::T2GUIRectangle()
	{
		m_pMaterial = TNULL;
	}

	void T2GUIRectangle::Render()
	{
		T2GUIRenderer* pRenderer = T2GUI::GetSingletonWeak()->GetRenderer();
		pRenderer->SetMaterial(m_pMaterial);
		pRenderer->SetColour(m_Color);

		float width = UnpackFloat(m_Width);
		float height = UnpackFloat(m_Height);

		TVector2 pos1{ width * -0.5f, height * -0.5f };
		TVector2 pos2{ pos1.x + width, pos1.y + height };

		pRenderer->RenderRectangle(pos1, pos2, m_UV1, m_UV2);
		T2GUIElement::Render();
	}

	void T2GUIRectangle::SetMaterial(T2GUIMaterial* pMaterial)
	{
		m_pMaterial = pMaterial;
		m_UV1 = { 0.0f, 0.0f };
		m_UV2 = { 1.0f, 1.0f };
		TTODO("Set some other fields");
	}
}
