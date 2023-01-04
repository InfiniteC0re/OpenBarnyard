#pragma once

#include "Toshi/T2gui/T2GUIRenderer.h"
#include "Toshi/T2gui/T2GUIElement.h"


namespace Toshi
{
	class T2GUIContext;

	class T2GUI
	{
		static TMemoryHeap* s_memheap;
		static T2GUI* s_instance;

	public:
		T2GUI();
		~T2GUI() = default;

		T2GUIRenderer* GetRenderer() { return m_renderer; }
		void SetRenderer(T2GUIRenderer* a_renderer) { m_renderer = a_renderer; }

		static void Open(TMemoryHeap* a_memheap);
		static void Close();

		void Tick(float a_tickrate);

		T2GUIRenderer* m_renderer; // 0x0
		T2GUIContext* m_guiContext; // 0x4
		T2GUIContext* m_guiContext2; // 0x8
	};

	class T2GUIContext : public T2GUIElement
	{
	public:
		T2GUIContext(T2GUI* a_gui);
	};
}
