#include "ToshiPCH.h"
#include "T2GUI.h"

Toshi::TMemoryHeap* Toshi::T2GUI::s_memheap = TNULL;

Toshi::T2GUI::T2GUI()
{
	m_renderer = TNULL;
	T2GUIContext* context = new T2GUIContext();
}

void Toshi::T2GUI::Open(TMemoryHeap* a_memheap)
{
	s_memheap = a_memheap;
	s_instance = new T2GUI();
}

void Toshi::T2GUI::Close()
{
	s_instance->~T2GUI();
	s_instance = TNULL;
}

void Toshi::T2GUI::Tick(float a_tickrate)
{
	if (m_guiContext != TNULL)
	{
		m_guiContext->Tick(a_tickrate);
	}
}

Toshi::T2GUIContext::T2GUIContext(T2GUI* a_gui) : Toshi::T2GUIElement::T2GUIElement()
{

}
