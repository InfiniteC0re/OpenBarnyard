#include "pch.h"
#include "ACFPBashHudElement.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/T2String16.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00450690, CreateObject 00450be0, CreateObjectInPlace 00450c40 (the constructor is not inlined)
TDEFINE_CLASS( ACFPBashHudElement );

static TFLOAT s_fDialHiddenY; // 0x00781f0c, where the dial starts bouncing in from
static TFLOAT s_fDialY;       // 0x00781f10

// $Barnyard: FUNCTION 004506a0
// $Barnyard: FUNCTION 00450c80
ACFPBashHudElement::Needle::~Needle()
{
}

// NOTE: folded with AGameTimeHudElement::ClockHand::PreRender (00452540)
void ACFPBashHudElement::Needle::PreRender()
{
	T2GUIElement::PreRender();
	T2GUI::GetRenderer()->PushTransform( m_oPivotTransform, TVector2::VEC_ZERO, TVector2::VEC_ZERO );
}

// NOTE: folded with AGameTimeHudElement::ClockHand::PostRender (00452570)
void ACFPBashHudElement::Needle::PostRender()
{
	T2GUI::GetRenderer()->PopTransform();
	T2GUIElement::PostRender();
}

// $Barnyard: FUNCTION 004506b0
ACFPBashHudElement::ACFPBashHudElement()
    : AHudElement( MASK_MINIGAME )
    , m_fValue( 0.0f )
    , m_fUnk18( -1.0f )
    , m_fUnk1C( -1.0f )
    , m_fUnk20( -1.0f )
    , m_fUnk24( -1.0f )
    , m_iUnk28( 0 )
    , m_fPopTextScale( 1.0f )
    , m_fPopTextAlpha( 1.0f )
    , m_fBouncePhase( 0.0f )
{
}

// $Barnyard: FUNCTION 00450780
// $Barnyard: FUNCTION 00450ca0
ACFPBashHudElement::~ACFPBashHudElement()
{
}

// $Barnyard: FUNCTION 00450cc0
TBOOL ACFPBashHudElement::OnAttach()
{
	// The dial
	m_oDial.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "dial_backing" ) );
	AGUI2::GetRootElement()->AddChildTail( m_oDial );
	m_oDial.Hide();

	// The needle rotates around its bottom
	m_oNeedle.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "needle" ) );
	m_oNeedle.GetTransform().SetTranslation( 0.0f, 300.0f );
	m_oNeedle.m_oPivotTransform.SetTranslation( m_oNeedle.GetWidth() * 0.5f, -300.0f );
	m_oDial.AddChildTail( m_oNeedle );

	// The popping text above the dial
	m_oPopText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 0.0f );
	m_oPopText.SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_oPopText.Hide();
	m_oDial.AddChildTail( m_oPopText );

	// The text below the dial
	m_oText.Create( T2GUIFontManager::FindFont( "Rekord18" ), 0.0f );
	m_oText.Hide();
	m_oDial.AddChildTail( m_oText );
	m_oText.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_oText.GetTransform().SetTranslation( 0.0f, 10.0f );
	m_oText.Hide();

	// The dial sits at the bottom centre of the screen
	s_fDialHiddenY = m_oDial.GetHeight() * 1.25f;
	s_fDialY       = s_fDialHiddenY - 20.0f;
	m_oDial.GetTransform().SetTranslation( 0.0f, s_fDialY );
	m_oDial.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );

	return TTRUE;
}

// $Barnyard: FUNCTION 00450810
void ACFPBashHudElement::OnDetach()
{
	m_oNeedle.RemoveSelf();
	m_oPopText.RemoveSelf();
	m_oText.RemoveSelf();
	m_oDial.RemoveSelf();
}

// $Barnyard: FUNCTION 00450860
void ACFPBashHudElement::Show()
{
	m_oDial.Show();
	m_oDial.GetTransform().SetTranslation( 0.0f, s_fDialHiddenY );
	m_fBouncePhase      = 0.0f;
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00450890
void ACFPBashHudElement::Hide()
{
	m_oDial.Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 004509b0
void ACFPBashHudElement::Update( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fBounceEnd   = TMath::HALF_PI + 0.2617994f; // 0x00781f54, 90 + 15 degrees
	static TFLOAT s_fMinAngle    = -0.40142572f;                // 0x00781f50, -23 degrees
	static TFLOAT s_fMaxAngle    = 0.34906584f;                 // 0x00781f4c, 20 degrees
	static TFLOAT s_fAngleRange  = s_fMaxAngle - s_fMinAngle;   // 0x00781f48
	static TFLOAT s_fNeedleAngle = s_fMinAngle;                 // 0x00781f44

	// Bounce the dial in (overshoots by 15 degrees of the sine)
	m_fBouncePhase += a_fDeltaTime * 5.0f;

	if ( m_fBouncePhase < s_fBounceEnd )
		m_oDial.GetTransform().SetTranslation( 0.0f, s_fDialY - TMath::Sin( m_fBouncePhase ) * 1.2f * s_fDialHiddenY );

	// Rotate the needle
	s_fNeedleAngle = s_fAngleRange * m_fValue + s_fMinAngle;
	TMath::Clip( s_fNeedleAngle, s_fMinAngle, s_fMaxAngle );

	T2GUITransform& rNeedleTransform = m_oNeedle.GetTransform();
	TVector2        vNeedlePos       = rNeedleTransform.GetTranslation();
	rNeedleTransform.SetIdentity();
	rNeedleTransform.Rotate( s_fNeedleAngle );
	rNeedleTransform.SetTranslation( vNeedlePos );

	if ( m_oPopText.IsVisible() )
	{
		// Scale the text up and fade it out once it's half way there
		m_fPopTextScale = TMath::LERP( m_fPopTextScale, 3.0f, a_fDeltaTime + a_fDeltaTime );

		T2GUITransform& rTextTransform = m_oPopText.GetTransform();
		TVector2        vTextPos       = rTextTransform.GetTranslation();
		rTextTransform.SetIdentity();
		rTextTransform.Scale( m_fPopTextScale, m_fPopTextScale );
		rTextTransform.SetTranslation( vTextPos );

		if ( 3.0f * 0.5f <= m_fPopTextScale )
			m_fPopTextAlpha = TMath::LERP( m_fPopTextAlpha, 0.0f, a_fDeltaTime * 10.0f );

		m_oPopText.SetAlpha( m_fPopTextAlpha );
	}
}

// $Barnyard: FUNCTION 004508b0
void ACFPBashHudElement::SetText( const TWCHAR* a_wszText )
{
	if ( a_wszText )
	{
		m_oText.SetTextAndFitWidth( a_wszText );
		m_oText.Show();
	}
	else
	{
		m_oText.Hide();
	}
}

// $Barnyard: FUNCTION 00450910
void ACFPBashHudElement::SetPopText( const TWCHAR* a_wszText )
{
	if ( a_wszText )
	{
		TSIZE uiLength = T2String16::Length( a_wszText );

		if ( T2String16::Compare( m_wszPopText, a_wszText, uiLength ) != 0 )
		{
			// NOTE: retail doesn't check the length against MAX_POP_TEXT_LENGTH
			ALocale::FormatInputTags( m_wszPopText, TINT( uiLength + 1 ), a_wszText );
			m_wszPopText[ uiLength ] = L'\0';

			m_oPopText.SetTextAndFitWidth( m_wszPopText );
			m_oPopText.Show();
			m_fPopTextAlpha = 1.0f;
			m_fPopTextScale = 1.0f;
		}
	}
	else
	{
		m_oPopText.Hide();
		m_fPopTextAlpha = 0.0f;
	}
}
