#pragma once
#include "AGUI2Style.h"
#include "AGUI2Transform.h"

#include <Toshi/Typedefs.h>
#include <Math/TVector2.h>

#include <Toshi/T2DList.h>

class AGUI2Element;

class AGUI2ElementNode
{
public:
	friend AGUI2Element;

public:
	AGUI2ElementNode()
	{
		m_pNext = this;
		m_pPrev = this;
	}

	// $Barnyard: FUNCTION 006c2c40
	virtual ~AGUI2ElementNode()
	{
		Unlink();
	}

	// $Barnyard: FUNCTION 006c2240
	void Unlink()
	{
		m_pNext->m_pPrev = m_pPrev;
		m_pPrev->m_pNext = m_pNext;
		m_pNext          = this;
		m_pPrev          = this;
	}

	// Sets a_rNode to be next after this
	void LinkAfter( AGUI2ElementNode& a_rNode )
	{
		m_pNext          = &a_rNode;
		m_pPrev          = a_rNode.m_pPrev;
		a_rNode.m_pPrev  = this;
		m_pPrev->m_pNext = this;
	}

	// Sets a_rNode to be previous to this
	void LinkBefore( AGUI2ElementNode& a_rNode )
	{
		m_pNext                  = a_rNode.m_pNext;
		m_pPrev                  = &a_rNode;
		a_rNode.m_pNext->m_pPrev = this;
		a_rNode.m_pNext          = this;
	}

	TBOOL             IsLinked() const { return this != m_pNext; }
	AGUI2ElementNode* GetNext() const { return m_pNext; }
	AGUI2ElementNode* GetPrev() const { return m_pPrev; }

protected:
	AGUI2ElementNode* m_pNext;
	AGUI2ElementNode* m_pPrev;
};

enum AGUI2ATTACHMENT
{
	AGUI2ATTACHMENT_TOPLEFT,
	AGUI2ATTACHMENT_TOPCENTER,
	AGUI2ATTACHMENT_TOPRIGHT,
	AGUI2ATTACHMENT_MIDDLELEFT,
	AGUI2ATTACHMENT_MIDDLECENTER,
	AGUI2ATTACHMENT_MIDDLERIGHT,
	AGUI2ATTACHMENT_BOTTOMLEFT,
	AGUI2ATTACHMENT_BOTTOMCENTER,
	AGUI2ATTACHMENT_BOTTOMRIGHT,
	AGUI2ATTACHMENT_NUMOF,
};

class AGUI2Element : public AGUI2ElementNode
{
public:
	using t_PostRender = void ( * )();

public:
	AGUI2Element();
	virtual ~AGUI2Element() = default;

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
	virtual TBOOL  IsPointInside( TFLOAT a_fX, TFLOAT a_fY );

	void AnchorPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight );
	void PivotPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight );

	void GetScreenTransform( AGUI2Transform& a_rOutTransform );
	void GetInvScreenTransform( AGUI2Transform& a_rOutTransform );

	void Show()
	{
		m_eFlags |= 1;
	}

	void Hide()
	{
		m_eFlags &= ~1;
	}

	// $Barnyard: FUNCTION 006c3ad0
	void SetInFront()
	{
		m_eFlags |= 16;
	}

	void SetRenderBeforeChildren()
	{
		m_eFlags |= 8;
	}

	// $Barnyard: FUNCTION 006c2ab0
	TBOOL HasFlags( TUINT32 a_uiFlags ) const
	{
		return ( m_eFlags & a_uiFlags ) == m_eFlags;
	}

	// $Barnyard: FUNCTION 00406c80
	TBOOL IsVisible() const
	{
		return ( m_eFlags & 1 ) && ( m_uiVisibilityMask & s_uiVisibilityMask ) != 0;
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

	TFLOAT GetAlpha() const
	{
		return ( ( m_uiColour & 0xFF000000 ) >> 24 ) * ( 1.0f / 255.0f );
	}

	// $Barnyard: FUNCTION 006c2c00
	void AddChildTail( AGUI2Element& a_rElement )
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkBefore( m_Children );
	}

	// $Barnyard: FUNCTION 006c2bc0
	void AddChildHead( AGUI2Element& a_rElement )
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkAfter( m_Children );
	}

	void SetTranslation( TFLOAT a_fX, TFLOAT a_fY )
	{
		m_oTransform.SetTranslation( a_fX, a_fY );
	}

	void SetTranslation( const Toshi::TVector2& a_rTranslation )
	{
		m_oTransform.SetTranslation( a_rTranslation );
	}

	// $Barnyard: FUNCTION 006c2590
	void SetTransform( TFLOAT a_fX, TFLOAT a_fY, float a_fRotAngle = 0.0f )
	{
		m_oTransform = AGUI2Transform();
		m_oTransform.Rotate( a_fRotAngle );
		m_oTransform.SetTranslation( a_fX, a_fY );
	}

	void SetAnchor( AGUI2ATTACHMENT a_eAnchor )
	{
		m_eAnchor = a_eAnchor;
	}

	void SetPivot( AGUI2ATTACHMENT a_ePivot )
	{
		m_ePivot = a_ePivot;
	}

	void SetAttachment( AGUI2ATTACHMENT a_eAnchor, AGUI2ATTACHMENT a_ePivot )
	{
		m_eAnchor = a_eAnchor;
		m_ePivot  = a_ePivot;
	}

	// $Barnyard: FUNCTION 006c2b00
	t_PostRender SetPostRenderCallback( t_PostRender a_cbNewCallback )
	{
		return std::exchange( m_cbPostRender, a_cbNewCallback );
	}

	AGUI2Transform& GetTransform()
	{
		return m_oTransform;
	}

	// Returns first children or TNULL
	AGUI2Element* ChildBegin() const
	{
		return ( m_Children.IsLinked() ) ? (AGUI2Element*)m_Children.GetNext() : TNULL;
	}

	// $Barnyard: FUNCTION 006c2280
	// Returns last children or TNULL
	AGUI2Element* ChildRBegin() const
	{
		return ( m_Children.IsLinked() ) ? (AGUI2Element*)m_Children.GetPrev() : TNULL;
	}

	// Returns next children or TNULL
	AGUI2Element* GetNextChild( AGUI2Element* a_pCurrentChild )
	{
		if ( a_pCurrentChild->GetNext() == &m_Children || a_pCurrentChild->GetNext() == a_pCurrentChild )
			return TNULL;

		return (AGUI2Element*)a_pCurrentChild->GetNext();
	}

	// Returns previous children or TNULL
	AGUI2Element* GetPrevChild( AGUI2Element* a_pCurrentChild )
	{
		if ( a_pCurrentChild->GetPrev() == &m_Children || a_pCurrentChild->GetPrev() == a_pCurrentChild )
			return TNULL;

		return (AGUI2Element*)a_pCurrentChild->GetPrev();
	}

	void RemoveSelf()
	{
		AGUI2ElementNode::Unlink();
		m_pParent = TNULL;
	}

	// $Barnyard: FUNCTION 006c2260
	void LinkChildBefore( AGUI2Element& a_rNewChildren, AGUI2Element& a_rInsertAfter )
	{
		a_rNewChildren.LinkBefore( a_rInsertAfter );
	}

public:
	inline static TUINT32 s_uiVisibilityMask = 0xFFFFFFFF;

protected:
	AGUI2Element*    m_pParent;
	AGUI2Transform   m_oTransform;
	AGUI2ElementNode m_Children;
	AGUI2ATTACHMENT  m_eAnchor;
	AGUI2ATTACHMENT  m_ePivot;
	TFLOAT           m_fWidth;
	TFLOAT           m_fHeight;
	TUINT32          m_uiColour;
	TUINT32          m_eFlags;
	t_PostRender     m_cbPostRender;
	TUINT32          m_uiVisibilityMask;
};
