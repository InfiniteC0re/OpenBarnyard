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
			return T2GUI::CreateSingleton();
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

		T2GUIElement* GetRootElement() const
		{
			return m_pContext1->GetRootElement();
		}

		void Tick(float deltaTime)
		{
			if (m_pContext1)
			{
				m_pContext1->Tick(deltaTime);
			}
		}

		void Render();

		static T2GUIMaterial* CreateMaterial(const char* a_materialName);
		static void DestroyMaterial(TMaterial* material);
		static TTexture* GetTexture(const char* a_textureName);

	private:
		static TMemoryHeap* s_pMemHeap;

	private:
		T2GUIRenderer* m_pRenderer;
		T2GUIContext* m_pContext1;
		T2GUIContext* m_pContext2;
	};
}