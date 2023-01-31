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

		void LinkBefore(T2GUIElementNode* next)
		{
			TASSERT(next->m_parent == this);
			T2GUIElementNode* nodeNext = next->m_next;
			m_next = nodeNext;
			m_parent = next;
			nodeNext->m_parent = this;
			m_parent->m_next = this;
		}

		void LinkAfter(T2GUIElementNode* previous)
		{
			TASSERT(previous->m_parent == this);
			m_next = previous;
			m_parent = previous->m_parent;
			previous->m_parent = this;
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

	class T2GUIElement : public T2GUIElementNode
	{
	public:
		enum FLAGS
		{
			Flags_Focus = 1,
			Flags_Visible,
			FLAGS_XUIELEMENT = 4
		};
		
	public:
		T2GUIElement() : T2GUIElementNode()
		{
			TASSERT(m_children.m_next == &m_children);
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
			a_node->LinkAfter(m_children.m_next);
		}
		void AddChildTail(T2GUIElement* a_node)
		{
			a_node->Unlink();
			a_node->m_children = *this;
			a_node->LinkBefore(m_children.m_next);
		}

		bool GetFlag(uint32_t flag) { return m_flags & flag; }
		void SetGlobalVisMask(uint32_t a_globalVisMask) { m_uiGlobalVisMask = a_globalVisMask; }

		// vtable sorted after globs
		virtual ~T2GUIElement() = default;
		virtual void Tick(float a_tickrate);
		virtual void SkipRender();
		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();
		virtual void GetDimensions(float& a_rWidth, float& a_rHeight) { a_rWidth = m_width; a_rHeight = m_height; }
		virtual float GetWidth() const { return m_width; }
		virtual float GetHeight() const { return m_height; }
		virtual void SetWidth(float a_width) { m_width = a_width; }
		virtual void SetHeight(float a_height) { m_height = a_height; }
		virtual void SetDimensions(float& a_rWidth, float& a_rHeight) { m_width = a_rWidth; m_height = a_rHeight; }
		virtual void GetMins(float& a_minX, float& a_minY);
		virtual void GetMaxs(float& a_maxX, float& a_maxY);
		virtual void SetColour(uint32_t a_colour) { m_colour = a_colour; }
		virtual void SetAlpha(float a_alpha) { m_colour &= ~0xFF; m_colour |= (uint8_t)(a_alpha * 255.0f); }
		virtual float GetAlpha() { return (m_colour & 0xFF) / 255.0f; }
		virtual void SetVisible(bool enable = true) { enable ? m_flags |= 1 : m_flags &= ~1; }
		virtual void SetFocus(bool enable = true) { enable ? m_flags |= 2 : m_flags &= ~2; }
		virtual bool IsPointInside(const TVector2& a_rVector2);
		virtual Toshi::TVector2 GetPivot() { return Toshi::TVector2::VEC_ZERO; }

	public:
		static inline uint32_t m_uiGlobalVisMask = -1;

	private:
		T2GUIElementNode m_children; // 0x20 deblob
		float m_width;               // 0x3C
		float m_height;              // 0x40
		uint32_t m_colour;           // 0x44
		uint8_t m_flags;             // 0x48
	};	
}
