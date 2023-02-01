#pragma once
#include "T2GUIElement.h"

namespace Toshi
{
	class T2GUIMaterial;

	class T2GUIRectangle : public T2GUIElement
	{
	public:
		T2GUIRectangle();

		virtual void Render() override;

		void Create(float width, float height)
		{
			m_Width = PackFloat(width);
			m_Height = PackFloat(height);
		}

		void SetMaterial(T2GUIMaterial* pMaterial);

	private:
		T2GUIMaterial* m_pMaterial;
		TVector2 m_UV1;
		TVector2 m_UV2;
	};
}