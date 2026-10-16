#pragma once
#include "T2GUITransform.h"
#include "Toshi/Typedefs.h"
#include "Toshi/T2DList.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

class T2GUIElement;

class T2GUIElementNode
{
public:
	friend T2GUIElement;

public:
	T2GUIElementNode()
	{
		m_pNext = this;
		m_pPrev = this;
	}

	// $Barnyard: FUNCTION 006c2c40
	virtual ~T2GUIElementNode()
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
	void LinkAfter( T2GUIElementNode& a_rNode )
	{
		m_pNext          = &a_rNode;
		m_pPrev          = a_rNode.m_pPrev;
		a_rNode.m_pPrev  = this;
		m_pPrev->m_pNext = this;
	}

	// Sets a_rNode to be previous to this
	void LinkBefore( T2GUIElementNode& a_rNode )
	{
		m_pNext                  = a_rNode.m_pNext;
		m_pPrev                  = &a_rNode;
		a_rNode.m_pNext->m_pPrev = this;
		a_rNode.m_pNext          = this;
	}

	TBOOL             IsLinked() const { return this != m_pNext; }
	T2GUIElementNode* GetNext() const { return m_pNext; }
	T2GUIElementNode* GetPrev() const { return m_pPrev; }

protected:
	T2GUIElementNode* m_pNext;
	T2GUIElementNode* m_pPrev;
};

enum T2GUIATTACHMENT
{
	T2GUIATTACHMENT_TOPLEFT,
	T2GUIATTACHMENT_TOPCENTER,
	T2GUIATTACHMENT_TOPRIGHT,
	T2GUIATTACHMENT_MIDDLELEFT,
	T2GUIATTACHMENT_MIDDLECENTER,
	T2GUIATTACHMENT_MIDDLERIGHT,
	T2GUIATTACHMENT_BOTTOMLEFT,
	T2GUIATTACHMENT_BOTTOMCENTER,
	T2GUIATTACHMENT_BOTTOMRIGHT,
	T2GUIATTACHMENT_NUMOF,
};

class T2GUIElement : public T2GUIElementNode
{
public:
	using t_PostRender = void ( * )();

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                   = 0,
		FLAGS_VISIBLE                = BITFLAG( 0 ),
		FLAGS_FOCUSED                = BITFLAG( 1 ),
		FLAGS_XUIELEMENT             = BITFLAG( 2 ), // set by AGUI2XUIElement
		FLAGS_RENDER_BEFORE_CHILDREN = BITFLAG( 3 ),
		FLAGS_INFRONT                = BITFLAG( 4 ),
	};

public:
	T2GUIElement();
	// $Barnyard: FUNCTION 006c2b70
	// $Barnyard: FUNCTION 006c2c80
	virtual ~T2GUIElement() {}

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
#ifdef BARNYARD_LATEST
	// Offset added to the pivot of elements that have FLAGS_XUIELEMENT, see PreRender
	// $BarnyardWiiJP: FUNCTION 80154230
	virtual const TVector2& GetPivot() { return TVector2::VEC_ZERO; } // +0x48
#endif

	void AnchorPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight );
	void PivotPos( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight );

	void GetScreenTransform( T2GUITransform& a_rOutTransform );
	void GetInvScreenTransform( T2GUITransform& a_rOutTransform );

	void Show()
	{
		m_eFlags |= FLAGS_VISIBLE;
	}

	void Hide()
	{
		m_eFlags &= ~FLAGS_VISIBLE;
	}

	// NOTE: the out of line copy is also the last slot (+0x48) of the AGUI2Feathers vtable (00739d78)
	// $Barnyard: FUNCTION 004048d0
	void SetVisible( TBOOL a_bVisible )
	{
		if ( a_bVisible ) Show();
		else Hide();
	}

	// $Barnyard: FUNCTION 006c3ad0
	void SetInFront()
	{
		m_eFlags |= FLAGS_INFRONT;
	}

	// $Barnyard: FUNCTION 006c2aa0
	void SetFlags( FLAGS a_uiFlags )
	{
		m_eFlags |= a_uiFlags;
	}

	void SetRenderBeforeChildren()
	{
		m_eFlags |= FLAGS_RENDER_BEFORE_CHILDREN;
	}

	// $Barnyard: FUNCTION 006c2ab0
	// $BarnyardWiiJP: FUNCTION 8049feb4
	TBOOL HasFlags( FLAGS a_uiFlags ) const
	{
		return ( m_eFlags & a_uiFlags ) == a_uiFlags;
	}

	// $Barnyard: FUNCTION 00406c80
	TBOOL IsVisible() const
	{
		return ( m_eFlags & FLAGS_VISIBLE ) && ( m_uiVisibilityMask & s_uiVisibilityMask ) != 0;
	}

	TBOOL IsFocused() const
	{
		return m_eFlags & FLAGS_FOCUSED;
	}

	TBOOL ShouldRenderBeforeChildren()
	{
		return m_eFlags & FLAGS_RENDER_BEFORE_CHILDREN;
	}

	TBOOL IsInFront() const
	{
		return m_eFlags & FLAGS_INFRONT;
	}

	TFLOAT GetAlpha() const
	{
		return ( ( m_uiColour & 0xFF000000 ) >> 24 ) * ( 1.0f / 255.0f );
	}

	// $Barnyard: FUNCTION 006c2c00
	void AddChildTail( T2GUIElement& a_rElement )
	{
		a_rElement.Unlink();
		a_rElement.m_pParent = this;
		a_rElement.LinkBefore( m_Children );
	}

	// $Barnyard: FUNCTION 006c2bc0
	void AddChildHead( T2GUIElement& a_rElement )
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

	// NOTE: an instance of it lives in the game code
	// $Barnyard: FUNCTION 0045a9d0
	void SetTransform( const T2GUITransform& a_rcTransform ) { m_oTransform = a_rcTransform; }

	// $Barnyard: FUNCTION 006c2590
	void SetTransform( TFLOAT a_fX, TFLOAT a_fY, float a_fRotAngle = 0.0f )
	{
		m_oTransform = T2GUITransform();
		m_oTransform.Rotate( a_fRotAngle );
		m_oTransform.SetTranslation( a_fX, a_fY );
	}

	void SetAnchor( T2GUIATTACHMENT a_eAnchor )
	{
		m_eAnchor = a_eAnchor;
	}

	void SetPivot( T2GUIATTACHMENT a_ePivot )
	{
		m_ePivot = a_ePivot;
	}

	void SetAttachment( T2GUIATTACHMENT a_eAnchor, T2GUIATTACHMENT a_ePivot )
	{
		m_eAnchor = a_eAnchor;
		m_ePivot  = a_ePivot;
	}

	// $Barnyard: FUNCTION 006c2b00
	t_PostRender SetPostRenderCallback( t_PostRender a_cbNewCallback )
	{
		return std::exchange( m_cbPostRender, a_cbNewCallback );
	}

	T2GUITransform& GetTransform()
	{
		return m_oTransform;
	}

	// Returns first children or TNULL
	T2GUIElement* ChildBegin() const
	{
		return ( m_Children.IsLinked() ) ? (T2GUIElement*)m_Children.GetNext() : TNULL;
	}

	// $Barnyard: FUNCTION 006c2280
	// Returns last children or TNULL
	T2GUIElement* ChildRBegin() const
	{
		return ( m_Children.IsLinked() ) ? (T2GUIElement*)m_Children.GetPrev() : TNULL;
	}

	// Returns next children or TNULL
	T2GUIElement* GetNextChild( T2GUIElement* a_pCurrentChild )
	{
		if ( a_pCurrentChild->GetNext() == &m_Children || a_pCurrentChild->GetNext() == a_pCurrentChild )
			return TNULL;

		return (T2GUIElement*)a_pCurrentChild->GetNext();
	}

	// Returns previous children or TNULL
	// $Barnyard: FUNCTION 006c22a0
	T2GUIElement* GetPrevChild( T2GUIElement* a_pCurrentChild )
	{
		if ( a_pCurrentChild->GetPrev() == &m_Children || a_pCurrentChild->GetPrev() == a_pCurrentChild )
			return TNULL;

		return (T2GUIElement*)a_pCurrentChild->GetPrev();
	}

	// $Barnyard: FUNCTION 004048b0
	void RemoveSelf()
	{
		T2GUIElementNode::Unlink();
		m_pParent = TNULL;
	}

	// $Barnyard: FUNCTION 006c2260
	void LinkChildBefore( T2GUIElement& a_rNewChildren, T2GUIElement& a_rInsertAfter )
	{
		a_rNewChildren.LinkBefore( a_rInsertAfter );
	}

	TUINT32 GetColour() const { return m_uiColour; }

public:
	inline static TUINT32 s_uiVisibilityMask = 0xFFFFFFFF;

protected:
	T2GUIElement*    m_pParent;
	T2GUITransform   m_oTransform;
	T2GUIElementNode m_Children;
	T2GUIATTACHMENT  m_eAnchor;
	T2GUIATTACHMENT  m_ePivot;
	TFLOAT           m_fWidth;
	TFLOAT           m_fHeight;
	TUINT32          m_uiColour;
	FLAGS            m_eFlags;
	t_PostRender     m_cbPostRender;
	TUINT32          m_uiVisibilityMask;
};

TOSHI_NAMESPACE_END
