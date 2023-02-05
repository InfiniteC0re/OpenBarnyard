#pragma once

#include "T2GUI.h"

namespace Toshi
{
	class T2GUIPolygon : public T2GUIElement
	{
	public:
		T2GUIPolygon()
		{
			m_iNumVerts = 0;
			m_pMaterial = TNULL;
			m_pVertices = TNULL;
			m_pUV = TNULL;
			m_bCalculatedDimensions = TFALSE;
		}

		~T2GUIPolygon()
		{
			T2GUI::DestroyMaterial(m_pMaterial);
		}

		virtual void Render();
		virtual void GetDimensions(float& width, float& height);
		virtual void SetDimensions(float width, float height);

		void Create(int numpoints);

		void SetMaterial(T2GUIMaterial* pMaterial)
		{
			m_pMaterial = pMaterial;
		}

		TVector2* GetVertices() const
		{
			return m_pVertices;
		}

		TVector2* GetUV() const
		{
			return m_pUV;
		}

	private:
		T2GUIMaterial* m_pMaterial;
		uint32_t m_iNumVerts;
		TVector2* m_pVertices;
		TVector2* m_pUV;
		bool m_bCalculatedDimensions;
	};
}