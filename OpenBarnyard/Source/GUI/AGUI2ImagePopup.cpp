#include "pch.h"
#include "AGUI2ImagePopup.h"

#include <GUI/T2GUIFontManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Pop curve: sin( POP_ANGLE * scale ) / sin( POP_ANGLE ) overshoots 1.0 before settling
static constexpr TFLOAT POP_ANGLE = TMath::PI * 0.75f;

// $Barnyard: FUNCTION 004ab1c0
AGUI2ImagePopup::AGUI2ImagePopup()
    : m_uiEffects( 0 )
{
}

// $Barnyard: FUNCTION 004ab230
// $Barnyard: FUNCTION 004a7d80
AGUI2ImagePopup::~AGUI2ImagePopup()
{
}

// $Barnyard: FUNCTION 0063c260
void AGUI2ImagePopup::Tick( TFLOAT a_fDeltaTime )
{
	TFLOAT fPulse;

	if ( ( m_uiEffects & ( 1 << EFFECT_PULSE ) ) && m_fTargetScale == m_fScale )
	{
		m_fPulsePhase += a_fDeltaTime * m_fPulseSpeed;
		fPulse = TMath::Sin( m_fPulsePhase ) * 0.1f + 1.0f;
	}
	else
	{
		fPulse = 1.0f;
	}

	// Pop in/out
	if ( m_fScale > 0.0f || m_fTargetScale != m_fScale )
	{
		TFLOAT fNorm = 1.0f / TMath::Sin( POP_ANGLE );

		if ( m_fTargetScale > m_fScale )
			m_fScale += a_fDeltaTime + a_fDeltaTime;
		else if ( m_fTargetScale < m_fScale )
			m_fScale -= a_fDeltaTime + a_fDeltaTime;

		TMath::Clip( m_fScale, 0.0f, 1.0f );

		m_oImage.GetTransform().SetIdentity();
		TFLOAT fPop   = TMath::Sin( POP_ANGLE * m_fScale ) * fNorm;
		TFLOAT fScale = fPop * fPulse;
		m_oImage.GetTransform().Scale( fScale, fScale );

		SetVisible( fPop > 0.0f );
	}

	// Flip the text
	if ( m_wszNextText && IsVisible() )
	{
		TFLOAT fTextScale = 1.0f;

		if ( m_fTextFlip < 1.0f )
		{
			m_fTextFlip += a_fDeltaTime + a_fDeltaTime;
			TMath::Clip( m_fTextFlip, 0.0f, 1.0f );

			fTextScale = TMath::Max( ( 1.0f - TMath::Sin( TMath::PI * m_fTextFlip ) ) * ( 0.8f / TMath::Sin( TMath::PI * 0.5f ) ), 0.0f );

			if ( fTextScale < 0.01f || m_fTextFlip > 0.7f )
			{
				// Swap the text while it's squashed
				m_oText.SetText( m_wszNextText );
				SetColour( m_uiNextTextColour );
			}
		}

		m_oText.GetTransform().SetIdentity();
		m_oText.GetTransform().Scale( fTextScale * fPulse, fPulse );
		m_oText.SetVisible( fTextScale > 0.01f );
	}

	// Flash between white and m_uiFlashColour
	if ( ( m_uiEffects & ( 1 << EFFECT_FLASH ) ) && IsVisible() )
	{
		m_fFlashPhase += a_fDeltaTime * m_fPulseSpeed * 0.3f;

		TFLOAT fAngle = TFLOAT( fmod( m_fFlashPhase, 1.0 ) ) * TMath::PI;
		while ( fAngle < 0.0f ) fAngle += TMath::PI;

		TFLOAT fSin   = TMath::Sin( fAngle );
		TFLOAT fWhite = ( 1.0f - fSin ) * 255.0f;

		TUINT8 uiR = TUINT8( ( ( m_uiFlashColour >> 16 ) & 0xFF ) * fSin + fWhite );
		TUINT8 uiG = TUINT8( ( ( m_uiFlashColour >> 8 ) & 0xFF ) * fSin + fWhite );
		TUINT8 uiB = TUINT8( ( m_uiFlashColour & 0xFF ) * fSin + fWhite );

		m_oImage.SetColour( 0xFF000000 | ( uiR << 16 ) | ( uiG << 8 ) | uiB );
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0063c220
void AGUI2ImagePopup::SetColour( TUINT32 a_uiColour )
{
	m_oText.SetColour( a_uiColour );
}

// $Barnyard: FUNCTION 0063c190
void AGUI2ImagePopup::SetShown( TBOOL a_bShown )
{
	if ( m_fTargetScale != 1.0f )
		m_fFlashPhase = 0.0f;

	m_fTextFlip    = 0.95f;
	m_fTargetScale = a_bShown ? 1.0f : 0.0f;
}

// $Barnyard: FUNCTION 0063c1e0
void AGUI2ImagePopup::SetShownInstantly( TBOOL a_bShown )
{
	if ( a_bShown )
	{
		Show();
		m_fTargetScale = 1.0f;
		m_fScale       = 1.0f;
	}
	else
	{
		Hide();
		m_fTargetScale = 0.0f;
		m_fScale       = 0.0f;
	}
}

// $Barnyard: FUNCTION 0063c230
void AGUI2ImagePopup::Create( T2GUITextureSection* a_pImage, TFLOAT a_fScale )
{
	m_oImage.SetTextureSection( *a_pImage );
	Setup();
	m_oTransform.Scale( a_fScale, a_fScale );
}

// $Barnyard: FUNCTION 0063c640
void AGUI2ImagePopup::SetEffect( TINT a_iEffect, TBOOL a_bEnable )
{
	if ( a_iEffect == EFFECT_PULSE )
		m_fPulsePhase = 1.0f;

	if ( a_bEnable )
		m_uiEffects |= ( 1 << a_iEffect );
	else
		m_uiEffects &= ~( 1 << a_iEffect );
}

// $Barnyard: FUNCTION 0063c030
void AGUI2ImagePopup::Setup()
{
	AddChildTail( m_oImage );
	m_oImage.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	SetDimensions( m_oImage.GetWidth(), m_oImage.GetHeight() );

	m_oText.SetFont( T2GUIFontManager::FindFont( "rekord26" ) );
	m_oText.SetText( L"" );
	m_oImage.AddChildTail( m_oText );
	m_oText.SetColour( 0xFFFF0500 );
	m_oText.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );

	m_fTargetScale     = 0.0f;
	m_fScale           = 0.0f;
	m_wszNextText      = TNULL;
	m_fTextFlip        = 0.0f;
	m_uiNextTextColour = 0;

	T2GUITransform oZero;

	m_oImage.GetTransform().SetIdentity();
	oZero.SetZero();
	m_oImage.GetTransform().PreMultiply( oZero );

	m_fPulseSpeed = 5.0f;
	Hide();

	m_oText.GetTransform().SetIdentity();
	oZero.SetZero();
	m_oText.GetTransform().PreMultiply( oZero );
	m_oText.Hide();

	m_uiFlashColour = 0xFFFF0000;
}
