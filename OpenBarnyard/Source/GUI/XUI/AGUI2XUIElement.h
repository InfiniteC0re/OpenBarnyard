#pragma once
#ifdef BARNYARD_LATEST

#  include "XURData.h"

#  include <GUI/T2GUIElement.h>
#  include <GUI/T2GUITransform.h>
#  include <Toshi/TObject.h>
#  include <Input/TInputInterface.h>

class AGUI2XUIResource;

//-----------------------------------------------------------------------------
// Base of every element of a XUI scene (Wii build).
// RTTI: AGUI2XUIElement : Toshi::T2GUIElement (at 0x0), Toshi::TObject (at 0x54).
// NOTE: CodeWarrior lists TObject first in the RTTI and appends the new virtuals after the
// TObject part of the vtable. T2GUIElement is declared first here so the layout matches.
// The element flag 4 marks XUI elements (see T2GUIElement::PreRender).
//-----------------------------------------------------------------------------
class AGUI2XUIElement
    : public Toshi::T2GUIElement
    , public Toshi::TObject
{
public:
	TDECLARE_CLASS( AGUI2XUIElement, Toshi::TObject );

	enum STATE : TUINT32
	{
		STATE_NORMAL  = 0,
		STATE_FOCUSED = 1,
		STATE_PRESSED = 2,
	};

	// Bits of the XUI Anchor property
	using ANCHOR = TUINT32;
	enum ANCHOR_ : ANCHOR
	{
		ANCHOR_NONE     = 0,
		ANCHOR_LEFT     = BITFLAG( 0 ),
		ANCHOR_TOP      = BITFLAG( 1 ),
		ANCHOR_RIGHT    = BITFLAG( 2 ),
		ANCHOR_BOTTOM   = BITFLAG( 3 ),
		ANCHOR_CENTER_X = BITFLAG( 4 ),
		ANCHOR_CENTER_Y = BITFLAG( 5 ),
	};

public:
	AGUI2XUIElement();
	~AGUI2XUIElement();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void                   Tick( TFLOAT a_fDeltaTime ) override;
	virtual void                   PreRender() override;
	virtual void                   PostRender() override;
	virtual void                   SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override;
	virtual void                   SetAlpha( TFLOAT a_fAlpha ) override;
	virtual void                   SetFocus( TBOOL a_bFocused ) override;
	virtual const Toshi::TVector2& GetPivot() override;

	//-----------------------------------------------------------------------------
	// Own virtuals. The offsets are from the start of the Wii vtable.
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ); // +0x6C

	// $BarnyardWiiJP: FUNCTION 8040f3a0
	// Returns TTRUE if the input was handled
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent ) { return TFALSE; } // +0x70

	virtual void PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ); // +0x74
	virtual void PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd );                         // +0x78
	virtual void StopAnimation();                                                             // +0x7C

	// $BarnyardWiiJP: FUNCTION 804156a8
	virtual void OnPress( TBOOL a_bEnded ) {} // +0x80

	virtual void UpdateTimelinePropFloat( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fT );     // +0x84
	virtual void UpdateTimelinePropValue( AGUI2XUIResource& a_rResource, TUINT32 a_uiProp, TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iIndex, TBOOL a_bKeyframeChanged, TFLOAT a_fT ); // +0x88

	TBOOL            CreateChildren( AGUI2XUIResource& a_rResource, XURXUIElementData* a_pData );
	AGUI2XUIElement* FindChildElement( const TWCHAR* a_wszId );
	void             UpdateAnchoring( const Toshi::TVector2& a_rcDelta );

	const TWCHAR* GetId() const { return m_wszId; }
	void          RestoreOriginalTransform() { m_oTransform = m_oOriginalTransform; }
	TUINT32       GetState() const { return m_eState; }

	static TUINT32 MakeARGB( TINT a_iR, TINT a_iG, TINT a_iB, TINT a_iA );
	static void    SplitARGB( TUINT32 a_uiColour, TINT& a_rA, TINT& a_rR, TINT& a_rG, TINT& a_rB );
	static TUINT32 InterpColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TFLOAT a_fT );

protected:
	const TWCHAR*         m_wszId;              // 0x58
	Toshi::TVector2       m_vecPivot;           // 0x5C
	TFLOAT                m_fZ;                 // 0x64 z of the Position property
	TFLOAT                m_fOpacity;           // 0x68
	ANCHOR                m_uiAnchor;           // 0x6C
	TUINT32               m_eState;             // 0x70 STATE
	TUINT32               m_uiBlendMode;        // 0x74
	TBOOL                 m_bTimelineRecursion; // 0x78 !DisableTimelineRecursion
	TUINT32               m_uiColorWriteFlags;  // 0x7C
	Toshi::T2GUITransform m_oOriginalTransform; // 0x80 the transform timelines start from
};

#endif // BARNYARD_LATEST
