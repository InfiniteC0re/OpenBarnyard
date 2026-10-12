#pragma once
#include "HUD/AHudElement.h"
#include "GUI/T2GUIString.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// TClass 00781ed8. A power bar ("Power_Bar_New")
// that fills up towards m_fTargetValue, with two brackets marking levels on it, a
// scrolling arrow that can point at either bracket, two tick/cross results and a
// text that fades out. The bar is vertical, or horizontal after SetOrientation( TTRUE ).
// AHudElement (mask 0x8004), vtable 0071bd98.
// Used by Cow Tipping (+0xef78) and 00474550, 0052d010, 00532de0, 00646880.
// ctor 0044fd70, dtor 0044f640, size 0x4cc.
//-----------------------------------------------------------------------------
class AButtonSlamHudElement
    : public AHudElement
{
public:
	TDECLARE_CLASS( AButtonSlamHudElement, AHudElement );

	static constexpr TINT NUM_RESULTS = 2;

public:
	AButtonSlamHudElement();
	~AButtonSlamHudElement();

	//-----------------------------------------------------------------------------
	// AHudElement
	//-----------------------------------------------------------------------------
	virtual TBOOL OnAttach() override;
	virtual void  Show() override;
	virtual void  Hide() override;
	virtual void  Update( TFLOAT a_fDeltaTime ) override;

	// Sets position of the first bracket on the bar (0.0f..1.0f), a negative value hides the bar
	void SetBracketPos( TFLOAT a_fPos );

	// Sets position of the second bracket on the bar (0.0f..1.0f), a negative value hides it
	void SetBracket2Pos( TFLOAT a_fPos );

	// Sets the start of the yellow band (m_fTargetBandStart), a negative value hides it
	// $Barnyard: FUNCTION 004713d0
	void SetTargetBandStart( TFLOAT a_fStart )
	{
		m_fTargetBandStart = a_fStart;
		m_oBandTarget.SetVisible( a_fStart >= 0.0f );
	}

	// Shows or hides both brackets
	void SetBracketsVisible( TBOOL a_bVisible );

	// Points the arrow at a bracket (0 or 1), -1 hides it
	void SetArrowTarget( TINT a_iBracket );

	// Shows a tick (or a cross) as result a_uiIndex, an index above 1 hides both
	void SetResult( TUINT a_uiIndex, TBOOL a_bTick );

	// Shows a_wszText (TNULL hides it) for a_fShowTime and then fades it out in a_fFadeTime
	void SetText( const TWCHAR* a_wszText, TFLOAT a_fShowTime, TFLOAT a_fFadeTime );

	// Sets alpha of the whole bar
	void SetAlpha( TFLOAT a_fAlpha );

	// After a_fDelay, fades the bar by a_fAlphaChange in a_fFadeTime.
	// A negative a_fDelay makes the bar opaque and disables fading.
	void StartFade( TFLOAT a_fDelay, TFLOAT a_fFadeTime, TFLOAT a_fAlphaChange );

	// Sets the textures of the band (a_szBand) and the bar orientation, a_szSize
	// is only used for the band thickness
	void SetOrientation( TBOOL a_bHorizontal, const TCHAR* a_szBand, const TCHAR* a_szSize );

private:
	// Sets alpha of every child of a_pElement and of their children
	static void SetChildrenAlpha( Toshi::T2GUIElement* a_pElement, TFLOAT a_fAlpha );

public:
	Toshi::T2GUIRectangle m_oBar;                    // 0x14
	Toshi::T2GUIRectangle m_oBracket;                // 0x8c
	Toshi::T2GUIRectangle m_oBand;                   // 0x104
	Toshi::T2GUIRectangle m_oBandTarget;             // 0x17c
	Toshi::T2GUIRectangle m_oBracket2;               // 0x1f4
	Toshi::T2GUIElement   m_oArrowRoot;              // 0x26c
	Toshi::T2GUIRectangle m_oArrow;                  // 0x2c0
	Toshi::T2GUIRectangle m_aResults[ NUM_RESULTS ]; // 0x338
	Toshi::T2GUIString    m_oText;                   // 0x428
	TFLOAT                m_fTargetValue;            // 0x48c, 0.0f..1.0f
	TFLOAT                m_fValue;                  // 0x490, goes towards m_fTargetValue
	TFLOAT                m_fTargetBandStart;        // 0x494, the yellow band shows m_fTargetValue - this
	TFLOAT                m_fBracketPos;             // 0x498
	TFLOAT                m_fBracket2Pos;            // 0x49c
	TFLOAT                m_fFadeDelay;              // 0x4a0, -1.0f when not fading
	TFLOAT                m_fFadeTime;               // 0x4a4
	TFLOAT                m_fTextShowTime;           // 0x4a8, -1.0f when faded out
	TFLOAT                m_fTextFadeTime;           // 0x4ac
	TFLOAT                m_fAlphaChange;            // 0x4b0
	TFLOAT                m_fFillSpeed;              // 0x4b4
	Toshi::TVector2       m_vBandUV2;                // 0x4b8, horizontal bar only
	Toshi::TVector2       m_vBandUV4;                // 0x4c0, horizontal bar only
	TBOOL                 m_bHorizontal;             // 0x4c8
};

TSTATICASSERT( sizeof( AButtonSlamHudElement ) == 0x4cc );
