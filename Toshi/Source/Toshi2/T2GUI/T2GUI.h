#pragma once
#include "T2GUIElement.h"
#include "T2GUIRenderer.h"
#include "T2GUIContext.h"

namespace Toshi
{
	class T2GUI : public TSingleton<T2GUI>
	{
	public:
		T2GUI();
		~T2GUI();

		static T2GUI* Open(TMemoryHeap* pMemoryHeap)
		{
			s_pMemHeap = pMemoryHeap;
			T2GUI::CreateSingleton();
		}

		static void Close()
		{
			T2GUI::DestroySingleton();
			s_pMemHeap = TNULL;
		}

		T2GUIRenderer* GetRenderer() const
		{
			return m_pRenderer;
		}

		void SetRenderer(T2GUIRenderer* pRenderer)
		{
			m_pRenderer = pRenderer;
		}

		void Render();

	private:
		static TMemoryHeap* s_pMemHeap;

	private:
		T2GUIRenderer* m_pRenderer;
		T2GUIContext* m_pContext1;
		T2GUIContext* m_pContext2;
	};
}