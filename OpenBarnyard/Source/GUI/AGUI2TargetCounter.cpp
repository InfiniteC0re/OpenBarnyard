#include "pch.h"
#include "AGUI2TargetCounter.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0064cbd0
AGUI2TargetCounter::AGUI2TargetCounter()
{
	m_fPaddingX    = 0.0f;
	m_fPaddingY    = 0.0f;
	m_eFormat      = FORMAT_VALUE;
	m_iMax         = 0;
	m_iTargetValue = 0;
	m_bPulsing     = TFALSE;
	m_fIconSize    = 64.0f;
	m_fScale       = 1.0f;
	m_eLayout      = LAYOUT_CENTRED;
	m_iValue       = -999;
	m_bPulseUp     = TTRUE;
}

// $Barnyard: FUNCTION 0064cc90
// $Barnyard: FUNCTION 0064cdd0
AGUI2TargetCounter::~AGUI2TargetCounter()
{
	m_oText.RemoveSelf();
	m_oIcon.RemoveSelf();
	m_oBacking.RemoveSelf();
}

void AGUI2TargetCounter::UpdateSize()
{
	TFLOAT fTextHeight    = m_oText.GetHeight();
	TFLOAT fBackingHeight = m_oBacking.GetHeight();
	TFLOAT fContentWidth  = m_oText.GetWidth() + m_oIcon.GetWidth();
	TFLOAT fBackingWidth  = m_oBacking.GetWidth();

	const TFLOAT* pHeight = &m_fIconSize;

	if ( fBackingHeight > fTextHeight )
	{
		if ( m_fIconSize <= fBackingHeight )
			pHeight = &fBackingHeight;
	}
	else if ( m_fIconSize <= fTextHeight )
	{
		pHeight = &fTextHeight;
	}

	const TFLOAT* pWidth = ( fBackingWidth >= fContentWidth ) ? &fBackingWidth : &fContentWidth;
	SetDimensions( m_fPaddingX + *pWidth, m_fPaddingY + *pHeight );
}

void AGUI2TargetCounter::UpdateText()
{
	if ( m_eFormat == FORMAT_VALUE )
		TStringManager::String16Format( m_wszText, MAX_TEXT_LENGTH, L"%i", m_iValue );
	else if ( m_eFormat == FORMAT_VALUE_OF_MAX )
		TStringManager::String16Format( m_wszText, MAX_TEXT_LENGTH, L"%i / %i", m_iValue, m_iMax );

	T2GUIFont* pFont   = T2GUIFontManager::FindFont( "Rekord26" );
	TFLOAT     fWidth  = pFont->GetTextWidth( m_wszText, 1.0f );
	TFLOAT     fHeight = pFont->GetTextHeightWrapped( m_wszText, fWidth, 1.0f );
	fWidth             = pFont->GetTextWidth( m_wszText, 1.0f );

	m_oText.Create( pFont, pFont->GetTextWidth( m_wszText, 1.0f ) );
	m_oText.SetText( m_wszText );
	m_oText.SetSize( fWidth, fHeight, TTRUE );

	UpdateSize();
}

// $Barnyard: FUNCTION 0064cd40
void AGUI2TargetCounter::SetLayout( LAYOUT_ a_eLayout )
{
	m_eLayout = a_eLayout;

	if ( a_eLayout == LAYOUT_ICON_LEFT )
	{
		m_oIcon.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLERIGHT );
		m_oText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLELEFT );
	}
	else if ( a_eLayout == LAYOUT_ICON_RIGHT )
	{
		m_oIcon.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLELEFT );
		m_oText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLERIGHT );
	}
	else if ( a_eLayout == LAYOUT_CENTRED )
	{
		m_oIcon.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		m_oText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	}
}

// $Barnyard: FUNCTION 0064cdf0
void AGUI2TargetCounter::SetIcon( const TCHAR* a_szIcon )
{
	T2GUITextureSection* pSection = T2GUITextureSectionManager::GetTextureSection( a_szIcon );

	if ( pSection )
	{
		m_oIcon.SetTextureSection( pSection );
		m_oIcon.Show();

		// Fit the section in m_fIconSize
		TFLOAT fSectionWidth  = TFLOAT( TMath::Abs( pSection->GetPoint2X() - pSection->GetPoint1X() ) );
		TFLOAT fSectionHeight = TFLOAT( TMath::Abs( pSection->GetPoint2Y() - pSection->GetPoint1Y() ) );
		TFLOAT fWidth;
		TFLOAT fHeight;

		if ( fSectionWidth > fSectionHeight )
		{
			fWidth  = m_fIconSize;
			fHeight = m_fIconSize / fSectionWidth * fSectionHeight;
		}
		else
		{
			fHeight = m_fIconSize;
			fWidth  = m_fIconSize / fSectionHeight * fSectionWidth;
		}

		m_oIcon.SetDimensions( fWidth, fHeight );
	}

	UpdateSize();
}

// $Barnyard: FUNCTION 0064cf60
void AGUI2TargetCounter::SetValue( TINT a_iValue, TBOOL a_bForce )
{
	if ( a_iValue == m_iValue && !a_bForce )
		return;

	m_iValue   = a_iValue;
	m_bPulsing = TTRUE;
	UpdateText();
}

// $Barnyard: FUNCTION 0064d4f0
void AGUI2TargetCounter::SetCount( TINT a_iValue, TBOOL a_bForce )
{
	SetValue( a_iValue, a_bForce );
	m_iTargetValue = a_iValue;
}

// $Barnyard: FUNCTION 0064d400
void AGUI2TargetCounter::SetBacking( const TCHAR* a_szSection )
{
	// NOTE: the overload taking a reference (006c3b00), it sizes the backing like the section
	m_oBacking.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( a_szSection ) );
	m_oBacking.Show();
	UpdateSize();
}

// $Barnyard: FUNCTION 0064d510
TBOOL AGUI2TargetCounter::Create( const TCHAR* a_szIcon )
{
	m_fTickTimer = 0.0f;
	SetIcon( a_szIcon );

	TINT iValue = m_iValue;
	m_eFormat   = FORMAT_VALUE;
	m_iMax      = 100;
	SetValue( iValue, TTRUE );
	m_iTargetValue = iValue;

	// Start at 0
	if ( m_iValue != 0 )
	{
		m_iValue   = 0;
		m_bPulsing = TTRUE;
		UpdateText();
	}

	m_iTargetValue = 0;

	AddChildTail( m_oIcon );
	AddChildTail( m_oText );
	m_oBacking.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oBacking.Hide();
	AddChildHead( m_oBacking );

	m_eLayout = LAYOUT_CENTRED;
	m_oIcon.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_oText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	return TTRUE;
}

// $Barnyard: FUNCTION 0064d110
void AGUI2TargetCounter::Tick( TFLOAT a_fDeltaTime )
{
	TFLOAT fPulseSpeed = 7.0f;
	TFLOAT fPulseScale = 2.0f;

	if ( m_iTargetValue != m_iValue )
	{
		m_fTickTimer -= a_fDeltaTime;

		if ( m_fTickTimer < 0.0f )
		{
			// Count one towards the target value
			m_fTickTimer += 0.036666669f;
			fPulseSpeed = 30.0f;
			fPulseScale = 1.1f;

			TINT iValue = m_iValue + ( ( m_iTargetValue > m_iValue ) ? 1 : -1 );

			if ( iValue != m_iValue )
			{
				m_iValue   = iValue;
				m_bPulsing = TTRUE;
				UpdateText();
			}
		}
	}

	if ( !m_bPulsing )
		return;

	if ( m_bPulseUp )
		m_fScale += fPulseSpeed * a_fDeltaTime;
	else
		m_fScale -= fPulseSpeed * a_fDeltaTime;

	if ( m_fScale >= fPulseScale )
	{
		m_fScale   = fPulseScale;
		m_bPulseUp = TFALSE;
	}

	if ( m_fScale <= 1.0f )
	{
		m_fScale   = 1.0f;
		m_bPulsing = TFALSE;
		m_bPulseUp = TTRUE;
	}

	if ( m_eFormat == FORMAT_VALUE )
	{
		m_oText.SetScale( m_fScale );
	}
	else if ( m_eFormat == FORMAT_VALUE_OF_MAX )
	{
		T2GUITransform& rTransform = m_oIcon.GetTransform();
		rTransform.Set( TVector2( 1.0f, 0.0f ), TVector2( 0.0f, 1.0f ), TVector2( 0.0f, 0.0f ) );
		rTransform.Scale( m_fScale, m_fScale );
	}
}
