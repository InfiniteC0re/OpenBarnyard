#pragma once
#include "AGUI2Transform.h"

#include <Toshi/Typedefs.h>
#include <Toshi/Math/TVector2.h>

#include <Toshi2/T2DList.h>

class AGUI2Element;

class AGUI2ElementNode : public Toshi::T2DList<AGUI2ElementNode>::Node
{
public:
	virtual ~AGUI2ElementNode() = default;

	AGUI2Element* Element() { return TREINTERPRETCAST(AGUI2Element*, this); }
	const AGUI2Element* Element() const { return TREINTERPRETCAST(const AGUI2Element*, this); }
};

class AGUI2ElementList : public Toshi::T2DList<AGUI2ElementNode>
{
public:
	virtual ~AGUI2ElementList() = default;
};

class AGUI2Element : public AGUI2ElementNode
{
public:
	enum class Anchor : TUINT
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

	enum class Pivot : TUINT
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
	virtual void SetUnknown(TFLOAT a_fValue);
	virtual void SetFocus(TBOOL a_bFocused);
	virtual TBOOL IsPointInside(const Toshi::TVector2& a_rPoint);

	TBOOL IsVisible() const
	{
		return m_eFlags & 1;
	}

	TBOOL IsFocused() const
	{
		return m_eFlags & 2;
	}

	void AddChildTail(AGUI2Element* a_pElement)
	{
		if (a_pElement->IsLinked()) a_pElement->Remove();
		a_pElement->m_pParent = this;
		m_Children.PushBack(a_pElement);
	}

	void AddChildHead(AGUI2Element* a_pElement)
	{
		if (a_pElement->IsLinked()) a_pElement->Remove();
		a_pElement->m_pParent = this;
		m_Children.PushFront(a_pElement);
	}

	void SetTransform(TFLOAT a_fX, TFLOAT a_fY, float a_fRotAngle)
	{
		m_oTransform = AGUI2Transform();
		m_oTransform.Rotate(a_fRotAngle);
		m_oTransform.SetPosition(a_fX, a_fY);
	}

	t_PostRender SetPostRenderCallback(t_PostRender a_cbNewCallback)
	{
		return std::exchange(m_cbPostRender, a_cbNewCallback);
	}

public:
	inline static TUINT32 s_uiVisibilityMask = 0xFFFFFFFF;

private:
	AGUI2Element* m_pParent;
	AGUI2Transform m_oTransform;
	AGUI2ElementList m_Children;
	Anchor m_eAnchor;
	Pivot m_ePivot;
	TFLOAT m_fWidth;
	TFLOAT m_fHeight;
	TUINT32 m_uiColour;
	TUINT32 m_eFlags;
	t_PostRender m_cbPostRender;
	TUINT32 m_uiVisibilityMask;
};
