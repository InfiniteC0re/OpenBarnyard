#include "pch.h"
#include "AGUI2AnythingPopup.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: same as the static helpers in ASimPlayerMainHudElement.cpp (00445e40) and ABikeSteer.cpp (004083c0)
static void ColourToFloats( TUINT32 a_uiColour, TFLOAT& a_rA, TFLOAT& a_rR, TFLOAT& a_rG, TFLOAT& a_rB )
{
	a_rB = TUINT8( a_uiColour ) * ( 1.0f / 255.0f );
	a_rG = TUINT8( a_uiColour >> 8 ) * ( 1.0f / 255.0f );
	a_rR = TUINT8( a_uiColour >> 16 ) * ( 1.0f / 255.0f );
	a_rA = TUINT8( a_uiColour >> 24 ) * ( 1.0f / 255.0f );
}

static TUINT32 PackColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return ( ( ( TUINT32( a_fGreen * 255.0f ) | ( ( TUINT32( a_fRed * 255.0f ) | ( TUINT32( a_fAlpha * 255.0f ) << 8 ) ) << 8 ) ) << 8 ) | TUINT32( a_fBlue * 255.0f ) );
}

static TUINT32 LerpColour( TUINT32 a_uiStart, TUINT32 a_uiEnd, TFLOAT a_fT )
{
	TFLOAT fA1, fR1, fG1, fB1;
	TFLOAT fA2, fR2, fG2, fB2;
	ColourToFloats( a_uiStart, fA1, fR1, fG1, fB1 );
	ColourToFloats( a_uiEnd, fA2, fR2, fG2, fB2 );

	return PackColour( TMath::LERP( fR1, fR2, a_fT ), TMath::LERP( fG1, fG2, a_fT ), TMath::LERP( fB1, fB2, a_fT ), TMath::LERP( fA1, fA2, a_fT ) );
}

// $Barnyard: FUNCTION 00641310
AGUI2AnythingPopup::AGUI2AnythingPopup()
{
	m_eMode         = MODE_FADE;
	m_fStartScale   = 0.1f;
	m_fPulseSpeed   = 0.5f;
	m_wszPopupText  = TNULL;
	m_fTime         = 0.0f;
	m_fDelay        = 0.0f;
	m_fHoldTime     = 0.0f;
	m_fScale        = 1.0f;
	m_fEndScale     = 1.0f;
	m_fDuration     = 1.0f;
	m_fMoveDuration = 1.0f;
	m_vStartPos     = TVector2::VEC_ZERO;
	m_vEndPos       = TVector2::VEC_ZERO;
	m_bCentered     = TTRUE;
	m_bWhite        = TTRUE;
	m_bAutoPosition = TTRUE;
}

// $Barnyard: FUNCTION 006413b0
AGUI2AnythingPopup::~AGUI2AnythingPopup()
{
	if ( m_wszPopupText )
		RemoveSelf();
}

// $Barnyard: FUNCTION 00641430
void AGUI2AnythingPopup::SetText( const TWCHAR* a_wszText, TUINT32 a_uiColour, T2GUIElement* a_pParent )
{
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	if ( m_wszPopupText != a_wszText )
	{
		if ( m_wszPopupText )
			RemoveSelf();

		if ( a_wszText )
		{
			Create( pFont, pFont->GetTextWidth( a_wszText ) );

			if ( m_bCentered )
				SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );

			AGUI2TextBox::SetText( a_wszText );

			T2GUIElement* pParent = a_pParent ? a_pParent : T2GUI::GetContext()->GetRootElement();

			if ( m_bAutoPosition )
				SetTransform( 0.0f, pParent->GetHeight() * 0.25f );

			pParent->AddChildTail( *this );
			Show();

			if ( !m_bWhite )
			{
				SetColour( a_uiColour );
			}
			else
			{
				SetAlpha( 1.0f );
				SetColour( 0xFFFFFFFF );
			}
		}
	}

	m_fTime         = 0.0f;
	m_wszPopupText  = a_wszText;
	m_uiEndColour   = a_uiColour;
	m_uiStartColour = a_uiColour;
	Update( 0.0f );
}

// $Barnyard: FUNCTION 00641540
const TVector2& AGUI2AnythingPopup::SetStartPosition( const TVector2& a_rcPos )
{
	TFLOAT fWidth, fHeight;
	T2GUI::GetContext()->GetRootElement()->GetDimensions( fWidth, fHeight );

	m_vStartPos.x = fWidth * a_rcPos.x * 0.25f;
	m_vStartPos.y = fHeight * a_rcPos.y * 0.25f;
	return a_rcPos;
}

// $Barnyard: FUNCTION 006415b0
void AGUI2AnythingPopup::Update( TFLOAT a_fDeltaTime )
{
	m_fDelay -= a_fDeltaTime;

	if ( m_fDelay <= 0.0f )
		m_fTime += a_fDeltaTime;
	else
		m_fTime = 0.0f;

	if ( !m_bAutoPosition )
		m_oTransform = T2GUITransform();

	if ( !m_wszPopupText )
		return;

	if ( !m_bWhite && m_uiStartColour != m_uiEndColour && m_fDuration != 0.0f )
		SetColour( LerpColour( m_uiStartColour, m_uiEndColour, TMath::Min( m_fTime, 1.0f ) ) );

	if ( m_eMode == MODE_PULSE )
	{
		TFLOAT fScale = TMath::Abs( TMath::Sin( m_fPulseSpeed * m_fTime * TMath::TWO_PI ) ) * ( m_fScale - m_fStartScale ) + m_fStartScale;
		m_oTransform.Scale( fScale, fScale );
		m_oTransform.SetTranslation( m_vStartPos );

		if ( m_fTime > m_fDuration )
			SetText( TNULL, 0x55000000, TNULL );
	}
	else if ( m_eMode == MODE_FADE )
	{
		if ( m_fTime > m_fDuration )
		{
			TFLOAT fAlpha = TUINT8( m_uiColour >> 24 ) * ( 1.0f / 255.0f ) - a_fDeltaTime * 5.0f;
			TMath::Clip( fAlpha, 0.0f, 1.0f );
			SetAlpha( fAlpha );

			if ( fAlpha <= 0.0f )
				SetText( TNULL, 0x55000000, TNULL );
		}
	}
	else if ( m_eMode == MODE_SCALE_AND_MOVE )
	{
		if ( m_fTime > m_fDuration )
		{
			m_fHoldTime -= a_fDeltaTime;

			if ( m_fHoldTime > 0.0f )
			{
				m_oTransform.Scale( m_fScale, m_fScale );
				m_oTransform.SetTranslation( m_vStartPos );
				SetColour( m_uiStartColour );
				m_fTime -= a_fDeltaTime;
				return;
			}

			TFLOAT fProgress = TMath::Min( 1.0f, ( m_fTime - m_fDuration ) / m_fMoveDuration );

			TVector2 vPos;
			vPos.Lerp( m_vStartPos, m_vEndPos, fProgress );

			TFLOAT fScale = ( m_fEndScale - m_fScale ) * fProgress + m_fScale;
			m_oTransform.SetTranslation( vPos );
			m_oTransform.Scale( fScale, fScale );

			if ( m_uiStartColour != m_uiEndColour )
				SetColour( LerpColour( m_uiStartColour, m_uiEndColour, fProgress ) );

			SetVisible( fScale > 0.0f );

			if ( 1.0f == fProgress || ( vPos.x == m_vEndPos.x && vPos.y == m_vEndPos.y ) )
				SetText( TNULL, 0x55000000, TNULL );
		}
		else
		{
			TFLOAT fScale = ( m_fScale - m_fStartScale ) * ( m_fTime / m_fDuration ) + m_fStartScale;
			m_oTransform.Scale( fScale, fScale );
			m_oTransform.SetTranslation( m_vStartPos );
			SetColour( m_uiStartColour );
			SetVisible( fScale > 0.0f );
		}
	}
}
