#pragma once
#include "T2GUITransform.h"
#include "Toshi2/T2DList.h"

namespace Toshi
{
	/*
	class T2GUIElementNode : public T2DList<T2GUIElementNode>::Node
	{
	public:
		virtual ~T2GUIElementNode() = default;
	};*/

	class T2GUIElement : public T2DList<T2GUIElement>::Node
	{
	public:
		static constexpr float FLOAT_QUALITY = 6;
		static constexpr float FLOAT_SCALE = 1 / FLOAT_QUALITY;

		using Float = uint16_t;

		using t_PostRender = void(*)();

		enum class Pivot : uint8_t
		{
			TopRight,
			TopCenter,
			TopLeft,
			MiddleRight,
			MiddleCenter,
			MiddleLeft,
			BottomRight,
			BottomCenter,
			BottomLeft
		};

		enum class Anchor : uint8_t
		{
			TopRight,
			TopCenter,
			TopLeft,
			MiddleRight,
			MiddleCenter,
			MiddleLeft,
			BottomRight,
			BottomCenter,
			BottomLeft
		};

		typedef uint8_t FLAGS;
		enum FLAGS_ : FLAGS
		{
			FLAGS_VISIBLE = BITFIELD(0),
			FLAGS_FOCUSED = BITFIELD(1),
			FLAGS_XUIELEMENT = BITFIELD(2),
		};

	public:
		T2GUIElement();
		virtual ~T2GUIElement();

		virtual void Tick(float deltaTime);
		virtual bool IsPaused() const;
		virtual bool SkipRender();
		virtual void PreRender();
		virtual void Render();
		virtual void PostRender();
		virtual void GetDimensions(float& width, float& height);
		virtual float GetWidth();
		virtual float GetHeight();
		virtual void SetWidth(float width);
		virtual void SetHeight(float height);
		virtual void SetDimensions(float width, float height);
		virtual void GetMins(float& x, float& y);
		virtual void GetMaxs(float& x, float& y);
		virtual void SetColour(uint32_t colour);
		virtual void SetAlpha(float alpha);
		virtual float GetAlpha();
		virtual void SetVisible(bool visible);
		virtual void SetFocus(bool focused);
		virtual void GetScreenTransform(void* unk);
		virtual bool IsPointInside(const TVector2& point);
		virtual const TVector2& GetPivot();

		void Unlink()
		{
			Node::Remove();
			m_pParent = TNULL;
		}

		void AddChildTail(T2GUIElement* pElement)
		{
			if (pElement->IsLinked()) pElement->Remove();
			pElement->m_pParent = this;
			m_Children.PushBack(pElement);
		}

		void AddChildHead(T2GUIElement* pElement)
		{
			if (pElement->IsLinked()) pElement->Remove();
			pElement->m_pParent = this;
			m_Children.PushFront(pElement);
		}

		void SetTransform(float x, float y, float angle)
		{
			m_Transform.Reset();
			m_Transform.GetPos().x = 0;
			m_Transform.GetPos().y = 0;
			m_Transform.Rotate(angle);
			m_Transform.GetPos().x = x;
			m_Transform.GetPos().y = y;
		}

		void SetAnchor(Anchor pivot)
		{
			m_Anchor = pivot;
		}

		void SetPivot(Pivot pivot)
		{
			m_Pivot = pivot;
		}

		static float UnpackFloat(Float value)
		{
			return value * FLOAT_SCALE;
		}

		static Float PackFloat(float value)
		{
			return static_cast<Float>(value * FLOAT_QUALITY);
		}

	protected:
		static constinit uint32_t s_uiGlobalVisMask;

	protected:
		T2GUIElement* m_pParent;
		T2GUITransform m_Transform;
		T2DList<T2GUIElement> m_Children;
		Float m_Width;
		Float m_Height;
		uint32_t m_Color;
		t_PostRender m_cbPostRender;
		FLAGS m_Flags1;                       // 0x3B
		Anchor m_Anchor;                      // 0x3C
		Pivot m_Pivot;                        // 0x3D
		FLAGS m_State;                        // 0x3E
	};
}