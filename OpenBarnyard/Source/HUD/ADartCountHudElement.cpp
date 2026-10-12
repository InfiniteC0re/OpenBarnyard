#include "pch.h"
#include "ADartCountHudElement.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ADartCountHudElement );

// Space between the icons (00781f9c, never set)
static TFLOAT s_fSpacing;

static TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue )
{
	TINT iRed   = TMath::FloorToInt( a_fRed * 255.0f );
	TINT iGreen = TMath::FloorToInt( a_fGreen * 255.0f );
	TINT iBlue  = TMath::FloorToInt( a_fBlue * 255.0f );

	return ( ( ( TUINT32( iRed ) | 0xFFFFFF00 ) << 8 | TUINT32( iGreen ) ) << 8 ) | TUINT32( iBlue );
}

// $Barnyard: FUNCTION 00451190
ADartCountHudElement::ADartCountHudElement()
    : AHudElement( MASK_UNK4 )
{
	m_fAlpha        = 1.0f;
	m_fFadeFrom     = 1.0f;
	m_fFadeTo       = 1.0f;
	m_fFadeTime     = 1.0f;
	m_fFadeDuration = 1.0f;
	m_pParent       = TNULL;
	m_pOnSection    = TNULL;
	m_pOffSection   = TNULL;
	m_bFlipped      = TFALSE;
	m_bFading       = TFALSE;
}

// $Barnyard: FUNCTION 00451510
// $Barnyard: FUNCTION 00451240
ADartCountHudElement::~ADartCountHudElement()
{
}

// $Barnyard: FUNCTION 00451680
TBOOL ADartCountHudElement::OnAttach()
{
	TFLOAT fWidth      = TMath::Abs( TFLOAT( m_pOnSection->GetPoint2X() - m_pOnSection->GetPoint1X() ) );
	TFLOAT fDartHeight = TMath::Abs( TFLOAT( m_pOnSection->GetPoint2Y() - m_pOnSection->GetPoint1Y() ) );

	m_oColumn.SetDimensions( fWidth, fDartHeight * 10.0f );
	m_oColumn.Show();
	m_oColumn.SetAlpha( 0.0f );
	m_oColumn.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oColumn.SetHeight( ( s_fSpacing + fDartHeight ) * 9.0f );

	for ( TINT i = 0; i < MAX_DARTS; i++ )
	{
		T2GUIRectangle& rDart = m_aDarts[ i ];

		rDart.SetDimensions( fWidth, fDartHeight );
		rDart.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		rDart.SetTextureSection( m_pOnSection );
		m_oColumn.AddChildTail( rDart );
		rDart.Show();
		rDart.SetAlpha( 1.0f );
		rDart.SetTransform( 20.0f, TFLOAT( i ) * fDartHeight );
	}

	m_pParent->AddChildTail( m_oColumn );
	SetCount( MAX_DARTS );

	return TTRUE;
}

// $Barnyard: FUNCTION 00451530
void ADartCountHudElement::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bFading == TTRUE )
	{
		TFLOAT fProgress = m_fFadeTime / m_fFadeDuration;
		TFLOAT fOne      = 1.0f;

		SetAlpha( TMath::LERP( m_fFadeFrom, m_fFadeTo, ( fProgress >= 1.0f ) ? fOne : fProgress ) );

		if ( m_fFadeTime <= m_fFadeDuration )
		{
			m_fFadeTime += a_fDeltaTime;
		}
		else
		{
			m_bFading = TFALSE;
			SetAlpha( m_fFadeTo );
		}
	}
}

// $Barnyard: FUNCTION 004512c0
void ADartCountHudElement::SetOnTexture( const TCHAR* a_szSection )
{
	m_pOnSection = T2GUITextureSectionManager::GetTextureSection( a_szSection );
}

// $Barnyard: FUNCTION 004512e0
void ADartCountHudElement::SetOffTexture( const TCHAR* a_szSection )
{
	m_pOffSection = T2GUITextureSectionManager::GetTextureSection( a_szSection );
}

// $Barnyard: FUNCTION 00451300
void ADartCountHudElement::SetAttachment( T2GUIATTACHMENT a_eAnchor, T2GUIATTACHMENT a_ePivot, TFLOAT a_fY )
{
	m_oColumn.SetAttachment( a_eAnchor, a_ePivot );

	if ( a_eAnchor == T2GUIATTACHMENT_TOPLEFT || a_eAnchor == T2GUIATTACHMENT_MIDDLELEFT )
		m_oColumn.SetTransform( 15.0f, a_fY );

	if ( a_eAnchor == T2GUIATTACHMENT_TOPRIGHT || a_eAnchor == T2GUIATTACHMENT_MIDDLERIGHT )
		m_oColumn.SetTransform( -( m_aDarts[ 0 ].GetWidth() * 0.5f + 15.0f ), a_fY );
}

// $Barnyard: FUNCTION 00451370
void ADartCountHudElement::SetColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue )
{
	TUINT32 uiColour = MakeColour( a_fRed, a_fGreen, a_fBlue );

	for ( TINT i = 0; i < MAX_DARTS; i++ )
		m_aDarts[ i ].SetColour( uiColour );
}

// $Barnyard: FUNCTION 004513f0
void ADartCountHudElement::SetAlpha( TFLOAT a_fAlpha )
{
	m_fAlpha = a_fAlpha;

	for ( TINT i = 0; i < MAX_DARTS; i++ )
	{
		// The used darts stay hidden
		if ( m_pOffSection && m_aDarts[ i ].GetMaterial() == m_pOffSection->GetMaterial() )
			m_aDarts[ i ].SetAlpha( 0.0f );
		else
			m_aDarts[ i ].SetAlpha( a_fAlpha );
	}
}

// $Barnyard: FUNCTION 00451450
void ADartCountHudElement::FadeTo( TFLOAT a_fAlpha, TFLOAT a_fDuration )
{
	m_fFadeFrom     = m_fAlpha;
	m_fFadeTo       = a_fAlpha;
	m_fFadeDuration = a_fDuration;
	m_fFadeTime     = 0.0f;
	m_bFading       = TTRUE;
}

// $Barnyard: FUNCTION 00451490
void ADartCountHudElement::FlipU()
{
	m_bFlipped = TTRUE;

	for ( TINT i = 0; i < MAX_DARTS; i++ )
		m_aDarts[ i ].FlipU();
}

// $Barnyard: FUNCTION 004514c0
void ADartCountHudElement::SetScale( TFLOAT a_fPercent )
{
	TFLOAT fScale = m_pParent->GetHeight() / TMath::Abs( m_oColumn.GetHeight() ) * ( a_fPercent * 0.01f );
	m_oColumn.GetTransform().Scale( fScale, fScale );
}

// $Barnyard: FUNCTION 004515e0
void ADartCountHudElement::SetCount( TINT a_iCount )
{
	TINT iNumUsed = MAX_DARTS - a_iCount;

	for ( TINT i = 0; i < iNumUsed; i++ )
	{
		m_aDarts[ i ].SetTextureSection( m_pOffSection );
		m_aDarts[ i ].SetAlpha( 0.0f );
	}

	for ( TINT i = ( iNumUsed > 0 ) ? iNumUsed : 0; i < MAX_DARTS; i++ )
		m_aDarts[ i ].SetTextureSection( m_pOnSection );

	if ( m_bFlipped )
		FlipU();
}
