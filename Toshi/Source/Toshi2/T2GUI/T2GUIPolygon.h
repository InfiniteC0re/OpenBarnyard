#pragma once

#include "T2GUI.h"

namespace Toshi
{
	class T2GUIPolygon : public T2GUIElement
	{
	public:
		T2GUIPolygon() : T2GUIElement()
		{
			m_iNumVerts = 0;
			m_pMaterial = TNULL;
			m_pUnk = TNULL;
			m_pUnk2 = TNULL;
		}

		virtual void Render();
		virtual void GetDimensions(float& width, float& height);
		virtual void SetDimensions(float width, float height)
		{
			TASSERT(TFALSE, "T2GUIPolygon::SetDimensions(): Can\'t set T2GUIPolygon dimensions directly!");
		}

		void Create(int numpoints);

		T2GUIMaterial* m_pMaterial;
		uint32_t m_iNumVerts;
		TVector2* m_pUnk;
		TVector2* m_pUnk2;
		bool m_bSomethingDimensions;
	};
}