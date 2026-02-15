#pragma once
#include <GUI/T2GUITransform.h>
#include "SDKHooks.h"

#include <Toshi/Typedefs.h>
#include <Math/TVector2.h>

#include <Toshi/T2DList.h>

class SDK_T2GUIElement;

class SDK_T2GUIElementNode
{
public:
	SDK_T2GUIElementNode()
	{
		m_pNext = this;
		m_pPrev = this;
	}

	virtual ~SDK_T2GUIElementNode()
	{
		Unlink();
	}

	void Unlink()
	{
		m_pNext->m_pPrev = m_pPrev;
		m_pPrev->m_pNext = m_pNext;
		m_pNext          = this;
		m_pPrev          = this;
	}

	void LinkAfter( SDK_T2GUIElementNode& a_rNode )
	{
		m_pNext          = &a_rNode;
		m_pPrev          = a_rNode.m_pPrev;
		a_rNode.m_pPrev  = this;
		m_pPrev->m_pNext = this;
	}

	void LinkBefore( SDK_T2GUIElementNode& a_rNode )
	{
		m_pNext                  = a_rNode.m_pNext;
		m_pPrev                  = &a_rNode;
		a_rNode.m_pNext->m_pPrev = this;
		a_rNode.m_pNext          = this;
	}

	SDK_T2GUIElement* GetNextElem() { return TREINTERPRETCAST( SDK_T2GUIElement*, m_pNext ); }
	SDK_T2GUIElement* GetPrevElem() { return TREINTERPRETCAST( SDK_T2GUIElement*, m_pPrev ); }

protected:
	SDK_T2GUIElementNode* m_pNext;
	SDK_T2GUIElementNode* m_pPrev;
};

class SDK_T2GUIElement : public SDK_T2GUIElementNode
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

	using t_PostRender = void ( * )();

public:
	SDK_T2GUIElement();
	virtual ~SDK_T2GUIElement() = default;

	virtual void   Tick( TFLOAT a_fDeltaTime );
	virtual void   PreRender();
	virtual void   Render();
	virtual void   PostRender();
	virtual void   GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );
	virtual TFLOAT GetWidth();
	virtual TFLOAT GetHeight();
	virtual void   SetWidth( TFLOAT a_fWidth );
	virtual void   SetHeight( TFLOAT a_fHeight );
	virtual void   SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight );
	virtual void   GetMins( TFLOAT& a_fX, TFLOAT& a_fY );
	virtual void   GetMaxs( TFLOAT& a_fX, TFLOAT& a_fY );
	virtual void   SetColour( TUINT32 a_uiColour );
	virtual void   SetAlpha( TFLOAT a_fAlpha );
	virtual void   SetShadowAlpha( TFLOAT a_fAlpha );
	virtual void   SetFocus( TBOOL a_bFocused );
	virtual TBOOL  IsPointInside( const Toshi::TVector2& a_rPoint );

	void GetScreenTransform( Toshi::T2GUITransform& a_rOutTransform );
	void GetInvScreenTransform( Toshi::T2GUITransform& a_rOutTransform );

	static SDK_T2GUIElement* CreateFromEngine();

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

	void IsInFront()
	{
		m_eFlags |= 8;
	}

	TBOOL HasFlags( UINT32 a_uiFlags ) const
	{
		return ( m_eFlags & a_uiFlags ) == m_eFlags;
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

	TBOOL ShoudResetZCoordinate() const
	{
		return m_eFlags & 16;
	}

	void AddChildTail( SDK_T2GUIElement& a_rElement )
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkBefore( m_Children );
	}

	void AddChildHead( SDK_T2GUIElement& a_rElement )
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkAfter( m_Children );
	}

	void SetTransform( TFLOAT a_fX, TFLOAT a_fY, float a_fRotAngle = 0.0f )
	{
		m_oTransform = Toshi::T2GUITransform();
		m_oTransform.Rotate( a_fRotAngle );
		m_oTransform.SetTranslation( a_fX, a_fY );
	}

	void SetAnchor( Anchor a_eAnchor )
	{
		m_eAnchor = a_eAnchor;
	}

	void SetPivot( Pivot a_ePivot )
	{
		m_ePivot = a_ePivot;
	}

	void SetAttachment( Anchor a_eAnchor, Pivot a_ePivot )
	{
		m_eAnchor = a_eAnchor;
		m_ePivot  = a_ePivot;
	}

	t_PostRender SetPostRenderCallback( t_PostRender a_cbNewCallback )
	{
		return std::exchange( m_cbPostRender, a_cbNewCallback );
	}

	Toshi::T2GUITransform& GetTransform()
	{
		return m_oTransform;
	}

public:
	inline static TUINT32* s_uiVisibilityMask = (TUINT32*)0x0077cf90;

protected:
	SDK_T2GUIElement*    m_pParent;
	Toshi::T2GUITransform   m_oTransform;
	SDK_T2GUIElementNode m_Children;
	Anchor           m_eAnchor;
	Pivot            m_ePivot;
	TFLOAT           m_fWidth;
	TFLOAT           m_fHeight;
	TUINT32          m_uiColour;
	TUINT32          m_eFlags;
	t_PostRender     m_cbPostRender;
	TUINT32          m_uiVisibilityMask;
};
