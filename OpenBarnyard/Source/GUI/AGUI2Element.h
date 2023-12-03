#pragma once
#include "AGUI2Transform.h"

#include <Toshi/Typedefs.h>
#include <Toshi/Math/TVector2.h>

#include <Toshi2/T2DList.h>

class AGUI2Element;

class AGUI2ElementNode
{
public:
	AGUI2ElementNode()
	{
		m_pNext = this;
		m_pPrev = this;
	}

	virtual ~AGUI2ElementNode()
	{
		Unlink();
	}

	void Unlink()
	{
		m_pNext->m_pPrev = m_pPrev;
		m_pPrev->m_pNext = m_pNext;
		m_pNext = this;
		m_pPrev = this;
	}

	void LinkAfter(AGUI2ElementNode& a_rNode)
	{
		m_pNext = &a_rNode;
		m_pPrev = a_rNode.m_pPrev;
		a_rNode.m_pPrev = this;
		m_pPrev->m_pNext = this;
	}

	void LinkBefore(AGUI2ElementNode& a_rNode)
	{
		m_pNext = a_rNode.m_pNext;
		m_pPrev = &a_rNode;
		a_rNode.m_pNext->m_pPrev = this;
		a_rNode.m_pNext = this;
	}

	AGUI2Element* GetNextElem() { return TREINTERPRETCAST(AGUI2Element*, m_pNext); }
	AGUI2Element* GetPrevElem() { return TREINTERPRETCAST(AGUI2Element*, m_pPrev); }

protected:
	AGUI2ElementNode* m_pNext;
	AGUI2ElementNode* m_pPrev;
};

class AGUI2Element : public AGUI2ElementNode
{
public:
	using Pivot = uint32_t;
	enum Pivot_ : Pivot
	{
		Pivot_TopLeft,
		Pivot_TopCenter,
		Pivot_TopRight,
		Pivot_MiddleLeft,
		Pivot_MiddleCenter,
		Pivot_MiddleRight,
		Pivot_BottomLeft,
		Pivot_BottomCenter,
		Pivot_BottomRight,
	};

	using Anchor = uint32_t;
	enum Anchor_ : Anchor
	{
		Anchor_TopLeft,
		Anchor_TopCenter,
		Anchor_TopRight,
		Anchor_MiddleLeft,
		Anchor_MiddleCenter,
		Anchor_MiddleRight,
		Anchor_BottomLeft,
		Anchor_BottomCenter,
		Anchor_BottomRight,
	};

	using t_PostRender = void(*)();

public:
	AGUI2Element();
	virtual ~AGUI2Element() = default;

	virtual void Tick(TFLOAT a_fDeltaTime);
	virtual void PreRender();
	virtual void Render();
	virtual void PostRender();
	virtual void GetDimensions(TFLOAT& a_rWidth, TFLOAT& a_rHeight);
	virtual TFLOAT GetWidth();
	virtual TFLOAT GetHeight();
	virtual void SetWidth(TFLOAT a_fWidth);
	virtual void SetHeight(TFLOAT a_fHeight);
	virtual void SetDimensions(TFLOAT a_fWidth, TFLOAT a_fHeight);
	virtual void GetMins(TFLOAT& a_fX, TFLOAT& a_fY);
	virtual void GetMaxs(TFLOAT& a_fX, TFLOAT& a_fY);
	virtual void SetColour(TUINT32 a_uiColour);
	virtual void SetAlpha(TFLOAT a_fAlpha);
	virtual void SetShadowAlpha(TFLOAT a_fAlpha);
	virtual void SetFocus(TBOOL a_bFocused);
	virtual TBOOL IsPointInside(const Toshi::TVector2& a_rPoint);

	void GetScreenTransform(AGUI2Transform& a_rOutTransform);

	void Show()
	{
		m_eFlags |= 1;
	}

	void Hide()
	{
		m_eFlags &= ~1;
	}

	void SetInFront()
	{
		m_eFlags |= 16;
	}

	void SetRenderBeforeChildren()
	{
		m_eFlags |= 8;
	}

	TBOOL HasFlags(UINT32 a_uiFlags) const
	{
		return (m_eFlags & a_uiFlags) == m_eFlags;
	}

	TBOOL IsVisible() const
	{
		return m_eFlags & 1;
	}

	TBOOL IsFocused() const
	{
		return m_eFlags & 2;
	}

	TBOOL ShouldRenderBeforeChildren()
	{
		return m_eFlags & 8;
	}

	TBOOL IsInFront() const
	{
		return m_eFlags & 16;
	}

	void AddChildTail(AGUI2Element& a_rElement)
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkBefore(m_Children);
	}

	void AddChildHead(AGUI2Element& a_rElement)
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkAfter(m_Children);
	}

	void SetTransform(TFLOAT a_fX, TFLOAT a_fY, float a_fRotAngle = 0.0f)
	{
		m_oTransform = AGUI2Transform();
		m_oTransform.Rotate(a_fRotAngle);
		m_oTransform.SetPosition(a_fX, a_fY);
	}

	void SetAnchor(Anchor a_eAnchor)
	{
		m_eAnchor = a_eAnchor;
	}

	void SetPivot(Pivot a_ePivot)
	{
		m_ePivot = a_ePivot;
	}

	void SetAttachment(Anchor a_eAnchor, Pivot a_ePivot)
	{
		m_eAnchor = a_eAnchor;
		m_ePivot = a_ePivot;
	}

	t_PostRender SetPostRenderCallback(t_PostRender a_cbNewCallback)
	{
		return std::exchange(m_cbPostRender, a_cbNewCallback);
	}

	AGUI2Transform& GetTransform()
	{
		return m_oTransform;
	}

public:
	inline static TUINT32 s_uiVisibilityMask = 0xFFFFFFFF;

protected:
	AGUI2Element* m_pParent;
	AGUI2Transform m_oTransform;
	AGUI2ElementNode m_Children;
	Anchor m_eAnchor;
	Pivot m_ePivot;
	TFLOAT m_fWidth;
	TFLOAT m_fHeight;
	TUINT32 m_uiColour;
	TUINT32 m_eFlags;
	t_PostRender m_cbPostRender;
	TUINT32 m_uiVisibilityMask;
};
