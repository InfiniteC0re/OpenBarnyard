#pragma once
#include <Toshi/Math/TVector2.h>
namespace Toshi
{
	class T2GUIElementNode
	{
	public:
		T2GUIElementNode* m_next; // 0x4
		T2GUIElementNode* m_parent; // 0x8
	
		virtual ~T2GUIElementNode()
		{
			Unlink();
		}

		// Unused in Globs
		bool IsLinked()
		{
			return m_next != this && m_parent != this;
		}

		void LinkBefore(T2GUIElementNode* a_node)
		{
			T2GUIElementNode* nodeNext = a_node->m_next;
			m_next = nodeNext;
			m_parent = a_node;
			nodeNext->m_parent = this;
			m_parent->m_next = this;
		}

		void LinkAfter(T2GUIElementNode* a_node)
		{
			m_next = a_node;
			m_parent = a_node->m_parent;
			a_node->m_parent = this;
			m_parent->m_next = this;
		}

		void Unlink()
		{
			m_next->m_parent = m_parent;
			m_parent->m_next = m_next;
			m_next = this;
			m_parent = this;
		}

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

		enum Flags
		{
			Flags_Focus = 1,
			Flags_Visible
		};

		T2GUIElementNode m_children; // 0x20

		T2GUIElementNode* m_unkNodes; // 0x28
		float m_width; // 0x3C
		float m_height; // 0x40
		uint32_t m_colour; // 0x44
		uint8_t m_flags; // 0x48

		uint32_t m_uiGlobalVisMask;

		T2GUIElement() : T2GUIElementNode()
		{
			TASSERT(m_children.m_next == &m_children, "");
			m_children = T2GUIElementNode();
			m_width = 1.0f;
			m_height = 1.0f;
			m_colour = -1;
			m_flags = Flags_Focus | Flags_Visible;
		}

		void AddChildHead(T2GUIElement* a_node)
		{
			a_node->Unlink();
			a_node->m_children = *this;
			a_node->LinkAfter(m_unkNodes);
		}
		void AddChildTail(T2GUIElement* a_node)
		{
			a_node->Unlink();
			a_node->m_children = *this;
			a_node->LinkBefore(m_unkNodes);
		}

		void SetColour(uint32_t a_colour) { m_colour = a_colour; }
		void SetDimensions(float a_rWidth, float a_rHeight)
		{
			m_width = a_rWidth;
			m_height = a_rHeight;
		}
		void SetFocus(bool enable = true) { enable ? m_flags |= 2 : m_flags &= ~2; }
		void SetVisible(bool enable = true) { enable ? m_flags |= 1 : m_flags &= ~1; }
		void SetGlobalVisMask(uint32_t a_globalVisMask) { m_uiGlobalVisMask = a_globalVisMask; }

		float GetWidth() const { return m_width; }
		float GetHeight() const { return m_height; }
		Toshi::TVector2 GetPivot() { return Toshi::TVector2::VEC_ZERO; }

		void GetDimensions(float& a_rWidth, float& a_rHeight)
		{
			a_rWidth = m_width;
			a_rHeight = m_height;
		}

		virtual void Tick(float a_tickrate);
	};

	
}


