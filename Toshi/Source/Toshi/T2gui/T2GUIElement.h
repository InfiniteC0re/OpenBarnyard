#pragma once
namespace Toshi2
{
	class T2GUIElementNode
	{
	public:
		T2GUIElementNode* m_next; // 0x4
		T2GUIElementNode* m_parent; // 0x8
	
		T2GUIElementNode()
		{
			m_next->m_parent = m_parent;
			m_next->m_next = m_next;
			m_next = this;
			m_parent = this;
		}
	};

	class T2GUIElement : T2GUIElementNode
	{
	public:
		T2GUIElementNode m_children; // 0x20


		T2GUIElement() : T2GUIElementNode()
		{
			TASSERT(m_children.m_next == &m_children, "");
			m_children = T2GUIElementNode();
		}
	};

	
}


