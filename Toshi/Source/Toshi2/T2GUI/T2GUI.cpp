#include "ToshiPCH.h"
#include "T2GUI.h"

namespace Toshi
{
	T2GUI::T2GUI()
	{
		m_pRenderer = TNULL;
		m_pContext1 = TNULL;
		m_pContext2 = TNULL;

		T2GUIContext* pContext = new T2GUIContext();
		m_pContext1 = pContext;
		m_pContext2 = pContext;
	}

	T2GUI::~T2GUI()
	{
		if (m_pContext2)
		{
			delete m_pContext2;
		}
	}

	void T2GUI::Render()
	{
		if (m_pContext1 && m_pRenderer)
		{
			m_pRenderer->BeginScene();

			auto pRootElement = m_pContext1->GetRootElement();
			pRootElement->PreRender();
			pRootElement->Render();
			pRootElement->PostRender();

			m_pRenderer->EndScene();
		}
	}
}