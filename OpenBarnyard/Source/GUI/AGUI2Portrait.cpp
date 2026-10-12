#include "pch.h"
#include "AGUI2Portrait.h"
#include "Locale/ALocaleManager.h"
#include "Sound/ASoundManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Math/TMathInline.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Sounds of the clock
static constexpr ASoundId CLOCK_SOUND_WARNING  = 172;
static constexpr ASoundId CLOCK_SOUND_CRITICAL = 173;
static constexpr ASoundId CLOCK_SOUND_TIME_UP  = 176;

// $Barnyard: FUNCTION 00646a00
AGUI2Portrait::AGUI2Portrait()
{
	m_iClockSound   = -1;
	m_iClockCue     = -1;
	m_iUnk3         = 0;
	m_fClockRatio   = 0.0f;
	m_fTickTimer    = 0.0f;
	m_fTimeUpTimer  = 0.0f;
	m_fTickRotation = 0.0f;
	m_fTickInterval = 0.0f;
	m_bFading       = TFALSE;
	m_eFlags        = FLAGS_NONE;
	m_afUnk1[ 0 ]   = 1.0f;
	m_afUnk1[ 1 ]   = 1.0f;
	m_afUnk1[ 2 ]   = 1.0f;
	m_afUnk1[ 3 ]   = 1.0f;
	m_bClockEnabled = TTRUE;
	m_iUnk2         = -999;
}

// $Barnyard: FUNCTION 00646b70
// $Barnyard: FUNCTION 006472e0
AGUI2Portrait::~AGUI2Portrait()
{
	m_oName.RemoveSelf();
	m_oHide.RemoveSelf();
	m_oBackground.RemoveSelf();
	m_oTicker.RemoveSelf();
	m_oClockRoot.RemoveSelf();
}

// $Barnyard: FUNCTION 00646b50
TFLOAT AGUI2Portrait::GetWidth()
{
	return m_oBackground.GetWidth();
}

// $Barnyard: FUNCTION 00646b60
TFLOAT AGUI2Portrait::GetHeight()
{
	return m_oBackground.GetHeight();
}

// $Barnyard: FUNCTION 00646cb0
void AGUI2Portrait::StartClockFade( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fDuration )
{
	m_fFadeTo   = a_fTo;
	m_fFadeFrom = a_fFrom;
	m_oClockPolygon1.SetAlpha( a_fFrom );
	m_oClockPolygon2.SetAlpha( a_fFrom );
	m_fFadeDuration = a_fDuration;
	m_fFadeTime     = 0.0f;
	m_bFading       = TTRUE;
}

// $Barnyard: FUNCTION 00646d10
void AGUI2Portrait::SetClockDuration( TFLOAT a_fDuration )
{
	m_fClockDuration = a_fDuration;
}

// $Barnyard: FUNCTION 00646d20
void AGUI2Portrait::SetScore( TINT a_iScore )
{
	m_oTicker.SetScore( a_iScore );
	m_oTicker.SetSpinning( TTRUE );
}

// $Barnyard: FUNCTION 00646d40
void AGUI2Portrait::SetName( const TWCHAR* a_wszName )
{
	TStringManager::String16Format( m_wszName, TARRAYSIZE( m_wszName ), L"%s", a_wszName );

	T2GUIFont* pFont  = T2GUIFontManager::FindFont( "Rekord26" );
	TFLOAT     fWidth = pFont->GetTextWidth( m_wszName, 1.0f );
	pFont->GetTextHeightWrapped( m_wszName, fWidth, 1.0f );
	m_afUnk1[ 3 ] = fWidth;

	m_oName.Create( pFont, pFont->GetTextWidth( m_wszName, 1.0f ) );
	m_oName.SetText( m_wszName );
	m_oName.SetScale( 0.8f );
}

// $Barnyard: FUNCTION 00646dd0
void AGUI2Portrait::Layout( FLAGS a_eFlags )
{
	m_eFlags = a_eFlags;

	if ( a_eFlags & FLAGS_LEFT )
	{
		m_oHide.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
		m_oWood.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		m_oName.SetTextAlign( T2GUIFont::TextAlign_Left );
		m_oTicker.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );

		if ( a_eFlags & FLAGS_STOPWATCH )
		{
			m_oTicker.SetTransform( m_oClockRoot.GetWidth(), -1.0f );
			m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
			m_oName.SetTextAlign( T2GUIFont::TextAlign_Left );
			m_oName.SetTransform( m_oClockRoot.GetWidth() * 0.85f, m_oHide.GetHeight() * 0.25f );
			m_oClockRoot.SetTransform( 0.0f, m_oClockRoot.GetHeight() * 0.12f );
			m_oBackground.SetTransform( m_oWood.GetWidth() * 0.05f, 0.0f );
		}
		else
		{
			m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
			m_oName.SetTransform( m_oHide.GetWidth(), m_oHide.GetHeight() * 0.25f );
			m_oTicker.SetTransform( m_oHide.GetWidth() * 0.75f, -1.0f );
		}

		m_oBackground.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
		m_oClockRoot.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	}

	if ( a_eFlags & FLAGS_RIGHT )
	{
		m_oHide.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
		m_oWood.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		m_oName.SetTextAlign( T2GUIFont::TextAlign_Right );
		m_oTicker.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );

		if ( a_eFlags & FLAGS_STOPWATCH )
		{
			m_oTicker.SetTransform( -m_oClockRoot.GetWidth(), -1.0f );
			m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
			m_oName.SetTextAlign( T2GUIFont::TextAlign_Right );
			m_oName.SetTransform( m_oClockRoot.GetWidth() * -0.85f, m_oHide.GetHeight() * 0.25f );
			m_oClockRoot.SetTransform( 0.0f, m_oClockRoot.GetHeight() * 0.12f );
			m_oBackground.SetTransform( m_oWood.GetWidth() * -0.05f, 0.0f );
		}
		else
		{
			m_oName.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
			m_oName.SetTransform( -m_oHide.GetWidth(), m_oHide.GetHeight() * -0.25f );
			m_oTicker.SetTransform( m_oHide.GetWidth() * -0.75f, -1.0f );
		}

		m_oBackground.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
		m_oClockRoot.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
		m_oHide.FlipU();
	}

	if ( ( a_eFlags & FLAGS_STOPWATCH ) && !( a_eFlags & FLAGS_2_DIGITS ) )
		m_oWood.SetWidth( m_oWood.GetWidth() * 1.1f );

	T2GUITransform oScale;
	oScale.Set( TVector2( 0.75f, 0.0f ), TVector2( 0.0f, 0.75f ), TVector2::VEC_ZERO );
	m_oTransform.PreMultiply( oScale );
}

// $Barnyard: FUNCTION 00647140
void AGUI2Portrait::SetNameColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue )
{
	TUINT32 uiRed   = TMath::FloorToInt( a_fRed * 255.0f );
	TUINT32 uiGreen = TMath::FloorToInt( a_fGreen * 255.0f );
	TUINT32 uiBlue  = TMath::FloorToInt( a_fBlue * 255.0f );

	m_oName.SetColour( ( ( ( TUINT8( uiRed ) | 0xFFFFFF00 ) << 8 | uiGreen ) << 8 ) | uiBlue );
}

// $Barnyard: FUNCTION 006471b0
void AGUI2Portrait::SetNameAlpha( TFLOAT a_fAlpha )
{
	m_oName.SetAlpha( a_fAlpha );
}

// $Barnyard: FUNCTION 006471c0
void AGUI2Portrait::Destroy()
{
	m_oTicker.RemoveSelf();
	m_oName.RemoveSelf();
	m_oHide.RemoveSelf();
	m_oBackground.RemoveSelf();
	m_oStopwatch.RemoveSelf();
	m_oClockRoot.RemoveSelf();
	m_oClockRed.RemoveSelf();
}

// $Barnyard: FUNCTION 00647240
void AGUI2Portrait::SetClockVisible( TBOOL a_bVisible )
{
	m_oClockPolygon2.SetVisible( a_bVisible == TTRUE );
	m_oClockPolygon1.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00647290
void AGUI2Portrait::SetClockEnabled( TBOOL a_bEnabled )
{
	m_bClockEnabled = a_bEnabled;
	m_oClockPolygon2.SetVisible( a_bEnabled == TTRUE );
	m_oClockPolygon1.SetVisible( a_bEnabled == TTRUE );
}

// $Barnyard: FUNCTION 00647320
void AGUI2Portrait::SetClockTime( TFLOAT a_fTime )
{
	// NOTE: the original looks these strings up without using them
	ALocaleManager::Interface()->GetString( 346 );
	ALocaleManager::Interface()->GetString( 509 );

	if ( !( m_eFlags & FLAGS_STOPWATCH ) )
		return;

	m_fClockRatio = a_fTime / m_fClockDuration;
	TMath::Clip( m_fClockRatio, 0.0f, 1.0f );

	TINT iSound = -1;

	if ( m_fClockRatio > CLOCK_WARNING_RATIO )
	{
		m_oClockPolygon1.SetColour( 0xFF00FF00 );
		m_oClockPolygon2.SetColour( 0xFF00FF00 );
		m_oClockPolygon2.SetAlpha( 0.5f );
		m_oClockPolygon1.SetAlpha( 0.5f );
	}
	else if ( m_fClockRatio > 0.0f )
	{
		iSound = ( m_fClockRatio > CLOCK_CRITICAL_RATIO ) ? CLOCK_SOUND_CRITICAL : CLOCK_SOUND_WARNING;
	}
	else
	{
		iSound = CLOCK_SOUND_TIME_UP;
	}

	if ( m_iClockSound != iSound )
	{
		m_iClockSound = iSound;
		ASoundManager::GetSingleton()->StopCue( m_iClockCue );

		if ( m_iClockSound != -1 )
			m_iClockCue = ASoundManager::GetSingleton()->PlayCue( m_iClockSound );

		m_oStopwatch.GetTransform().SetIdentity();
		m_oStopwatch.GetTransform().Scale( STOPWATCH_SCALE, STOPWATCH_SCALE );
		m_oStopwatch.GetTransform().Rotate( m_fTickRotation );
		m_fTickRotation = ( m_fTickRotation == -TICK_ROTATION ) ? TICK_ROTATION : -TICK_ROTATION;
		m_fTickTimer    = 0.0f;

		if ( m_iClockSound != CLOCK_SOUND_TIME_UP )
		{
			TUINT32 uiColour = ( m_iClockSound == CLOCK_SOUND_CRITICAL ) ? 0xFFFFC800 : 0xFFFF0500;
			m_oClockPolygon1.SetColour( uiColour );
			m_oClockPolygon2.SetColour( uiColour );
			m_oClockPolygon2.SetAlpha( 0.5f );
			m_oClockPolygon1.SetAlpha( 0.5f );
		}
	}

	TFLOAT fAngle           = TMath::TWO_PI * m_fClockRatio;
	TFLOAT fSecondHalfAngle = fAngle - TMath::PI;

	if ( fSecondHalfAngle < 0.0f )
	{
		m_oClockPolygon1.Hide();
	}
	else
	{
		m_oClockPolygon1.Show();

		TVector2* pUVs = m_oClockPolygon1.GetUVs();
		pUVs[ 0 ].Set( 0.5f, 1.0f );
		pUVs[ 2 ].Set( 0.5f, 0.5f );

		if ( fSecondHalfAngle >= TMath::PI * 0.25f )
		{
			pUVs[ 3 ].x = 1.0f;
			pUVs[ 1 ].Set( 1.0f, 1.0f );

			if ( fSecondHalfAngle < TMath::PI * 0.75f )
			{
				pUVs[ 4 ].x = 1.0f;
				pUVs[ 3 ].y = ( TMath::Tan( TMath::PI * 0.5f - fSecondHalfAngle ) + 1.0f ) * 0.5f;
				pUVs[ 4 ].y = pUVs[ 3 ].y;
			}
			else
			{
				pUVs[ 3 ].y = 0.0f;
				pUVs[ 4 ].y = 0.0f;
				pUVs[ 4 ].x = ( TMath::Tan( TMath::PI - fSecondHalfAngle ) + 1.0f ) * 0.5f;
			}
		}
		else
		{
			pUVs[ 1 ].y = 1.0f;
			pUVs[ 3 ].y = 1.0f;
			pUVs[ 4 ].y = 1.0f;

			TFLOAT fX   = ( TMath::Tan( fSecondHalfAngle ) + 1.0f ) * 0.5f;
			pUVs[ 1 ].x = fX;
			pUVs[ 3 ].x = fX;
			pUVs[ 4 ].x = fX;
		}
	}

	TVector2* pUVs = m_oClockPolygon2.GetUVs();
	pUVs[ 0 ].Set( 0.5f, 0.0f );
	pUVs[ 2 ].Set( 0.5f, 0.5f );

	if ( fAngle > TMath::PI )
	{
		pUVs[ 1 ].x = 0.0f;
		pUVs[ 3 ].Set( 0.0f, 1.0f );
		pUVs[ 4 ].Set( 0.5f, 1.0f );
		pUVs[ 1 ].y = 0.0f;
	}
	else if ( fAngle > TMath::PI * 0.75f )
	{
		pUVs[ 1 ].Set( 0.0f, 0.0f );
		pUVs[ 3 ].Set( 0.0f, 1.0f );
		pUVs[ 4 ].Set( ( 1.0f - TMath::Tan( TMath::PI - fAngle ) ) * 0.5f, 1.0f );
	}
	else if ( fAngle > TMath::PI * 0.25f )
	{
		pUVs[ 1 ].Set( 0.0f, 0.0f );
		pUVs[ 3 ].x = 0.0f;
		pUVs[ 4 ].x = 0.0f;
		pUVs[ 3 ].y = ( TMath::Tan( fAngle - TMath::PI * 0.5f ) + 1.0f ) * 0.5f;
		pUVs[ 4 ].y = pUVs[ 3 ].y;
	}
	else
	{
		pUVs[ 3 ].y = 0.0f;
		pUVs[ 4 ].y = 0.0f;

		TFLOAT fX   = ( 1.0f - TMath::Tan( fAngle ) ) * 0.5f;
		pUVs[ 1 ].x = fX;
		pUVs[ 3 ].x = fX;
		pUVs[ 4 ].x = fX;
		pUVs[ 1 ].y = 0.0f;
	}

	// The vertices follow the UVs
	TVector2*       pVertices = m_oClockPolygon1.GetVertices();
	const TVector2* pSrcUVs   = m_oClockPolygon1.GetUVs();

	for ( TINT i = 0; i < m_oClockPolygon1.GetNumVertices(); i++ )
	{
		pVertices[ i ].x = ( pSrcUVs[ i ].x - 0.5f ) * m_fClockWidth * 1.02f;
		pVertices[ i ].y = ( pSrcUVs[ i ].y - 0.5f ) * m_fClockHeight * 1.02f;
	}

	pVertices = m_oClockPolygon2.GetVertices();
	pSrcUVs   = m_oClockPolygon2.GetUVs();

	for ( TINT i = 0; i < m_oClockPolygon2.GetNumVertices(); i++ )
	{
		pVertices[ i ].x = ( pSrcUVs[ i ].x - 0.5f ) * m_fClockWidth * 1.02f;
		pVertices[ i ].y = ( pSrcUVs[ i ].y - 0.5f ) * m_fClockHeight * 1.02f;
	}
}

// $Barnyard: FUNCTION 006477d0
void AGUI2Portrait::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bFading == TTRUE )
	{
		TFLOAT fAlpha = TMath::LERP( m_fFadeFrom, m_fFadeTo, TMath::Min( m_fFadeTime / m_fFadeDuration, 1.0f ) );
		m_oClockPolygon2.SetAlpha( fAlpha );
		m_oClockPolygon1.SetAlpha( fAlpha );

		if ( m_fFadeTime > m_fFadeDuration )
		{
			m_oClockPolygon2.SetAlpha( m_fFadeTo );
			m_oClockPolygon1.SetAlpha( m_fFadeTo );
			m_bFading = TFALSE;
		}

		m_fFadeTime += a_fDeltaTime;
	}

	if ( m_bClockEnabled == TTRUE && m_fClockRatio <= CLOCK_WARNING_RATIO )
	{
		// The stopwatch ticks faster the less time there's left
		if ( m_fClockRatio > 0.0f )
		{
			m_fTimeUpTimer  = 0.0f;
			m_fTickInterval = ( m_fClockRatio > CLOCK_CRITICAL_RATIO ) ? 0.25f : 0.1f;
		}
		else
		{
			m_fTickInterval = 0.01f;
			m_fTimeUpTimer += a_fDeltaTime;
		}

		if ( m_fTimeUpTimer > TIME_UP_SHAKE_TIME )
		{
			m_oStopwatch.GetTransform().SetIdentity();
			m_oStopwatch.GetTransform().Scale( STOPWATCH_SCALE, STOPWATCH_SCALE );
		}
		else
		{
			m_fTickTimer += a_fDeltaTime;

			if ( m_fTickTimer >= m_fTickInterval )
			{
				m_fTickTimer = 0.0f;
				m_oStopwatch.GetTransform().SetIdentity();
				m_oStopwatch.GetTransform().Scale( STOPWATCH_SCALE, STOPWATCH_SCALE );
				m_oStopwatch.GetTransform().Rotate( m_fTickRotation );
				m_fTickRotation = ( m_fTickRotation == -TICK_ROTATION ) ? TICK_ROTATION : -TICK_ROTATION;
			}
		}
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00647a40
void AGUI2Portrait::ResetClock( TBOOL a_bResetScore )
{
	ASoundManager::GetSingleton()->StopCue( m_iClockCue );
	m_oClockPolygon1.SetColour( 0xFF00FF00 );
	m_oClockPolygon2.SetColour( 0xFF00FF00 );
	m_oClockPolygon2.SetAlpha( 0.5f );
	m_oClockPolygon1.SetAlpha( 0.5f );

	m_fTickRotation = 0.0f;
	m_oStopwatch.GetTransform().SetIdentity();
	m_oStopwatch.GetTransform().Scale( STOPWATCH_SCALE, STOPWATCH_SCALE );
	m_fTickTimer   = 0.0f;
	m_fTimeUpTimer = 0.0f;
	m_iClockSound  = -1;

	if ( a_bResetScore == TTRUE )
		m_oTicker.Reset();
}

// $Barnyard: FUNCTION 00647b00
void AGUI2Portrait::CreateClock( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_fClockHeight = a_fHeight;
	m_fClockWidth  = a_fWidth;

	m_oClockPolygon1.CreateBuffers( 5 );
	m_oClockPolygon1.SetMaterial( T2GUITextureSectionManager::GetTextureSection( "clockyellow" )->GetMaterial() );
	m_oClockPolygon1.SetAlpha( 0.5f );

	m_oClockPolygon2.CreateBuffers( 5 );
	m_oClockPolygon2.SetMaterial( T2GUITextureSectionManager::GetTextureSection( "clockyellow" )->GetMaterial() );
	m_oClockPolygon2.SetAlpha( 0.5f );

	m_oHide.AddChildTail( m_oClockPolygon2 );
	m_oHide.AddChildTail( m_oClockPolygon1 );

	m_fClockDuration = 1.0f;
	SetClockTime( 1.0f );
}

// $Barnyard: FUNCTION 00647bb0
void AGUI2Portrait::SetupStopwatch()
{
	m_oStopwatch.SetAlpha( 1.0f );
	m_oStopwatch.GetTransform().SetIdentity();
	m_oStopwatch.GetTransform().Scale( STOPWATCH_SCALE, STOPWATCH_SCALE );

	TFLOAT fScale = 1.0f / STOPWATCH_SCALE;
	m_oHide.GetTransform().Scale( fScale, fScale );
	m_oHide.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	m_oClockRed.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "clockred" ) );
	m_oClockRed.SetAlpha( 0.75f );

	T2GUITransform oScale;
	oScale.Set( TVector2( 1.1f, 0.0f ), TVector2( 0.0f, 1.1f ), TVector2::VEC_ZERO );
	m_oClockRed.GetTransform().PreMultiply( oScale );

	CreateClock( m_oStopwatch.GetWidth(), m_oStopwatch.GetWidth() );
}

// $Barnyard: FUNCTION 00647cc0
TBOOL AGUI2Portrait::Create( const TCHAR* a_szHideTexture, FLAGS a_eFlags, T2GUIElement* a_pUnused )
{
	m_iNumDigits = 3;

	if ( a_eFlags & FLAGS_2_DIGITS )
		m_iNumDigits = 2;
	else if ( a_eFlags & FLAGS_4_DIGITS )
		m_iNumDigits = 4;

	m_oTicker.Create( m_iNumDigits, "darts_wheel_window", 1.0f, 8.0f );
	m_oTicker.Show();
	m_oTicker.SetSpinning( TFALSE );
	m_oTicker.SetScore( 0 );

	TFLOAT fTickerHeight = m_oTicker.GetHeight();

	T2GUITextureSection* pWood  = T2GUITextureSectionManager::GetTextureSection( "Score_Wood" );
	TFLOAT               fWoodW = TMath::Abs( TFLOAT( pWood->GetPoint2X() - pWood->GetPoint1X() ) );
	TFLOAT               fWoodH = TMath::Abs( TFLOAT( pWood->GetPoint2Y() - pWood->GetPoint1Y() ) );

	T2GUITextureSection* pHide  = T2GUITextureSectionManager::GetTextureSection( a_szHideTexture );
	TFLOAT               fHideW = TMath::Abs( TFLOAT( pHide->GetPoint2X() - pHide->GetPoint1X() ) );
	TFLOAT               fHideH = TMath::Abs( TFLOAT( pHide->GetPoint2Y() - pHide->GetPoint1Y() ) );

	m_oHide.T2GUIElement::SetDimensions( fHideW, fHideH );
	m_oHide.SetWidth( fHideW );
	m_oHide.SetHeight( fHideH );
	m_oHide.SetTextureSection( pHide );

	m_oBackground.SetAlpha( 0.0f );

	m_oName.Create( T2GUIFontManager::FindFont( "Rekord26" ), TMath::Max( fWoodW - fHideW, 100.0f ) );
	m_oName.SetScale( 0.8f );

	if ( a_eFlags & FLAGS_NO_NAME )
		m_oName.Hide();

	m_oWood.T2GUIElement::SetDimensions( fHideW * 0.5f + fWoodW, fWoodH );
	m_oWood.SetTextureSection( pWood );

	m_oBackground.T2GUIElement::SetDimensions( fHideW * 0.75f + fWoodW, fWoodH * 0.2f + fWoodH );

	T2GUITextureSection* pStopwatch = T2GUITextureSectionManager::GetTextureSection( "stopwatch" );
	m_oStopwatch.T2GUIElement::SetDimensions( TMath::Abs( TFLOAT( pStopwatch->GetPoint2X() - pStopwatch->GetPoint1X() ) ), TMath::Abs( TFLOAT( pStopwatch->GetPoint2Y() - pStopwatch->GetPoint1Y() ) ) );
	m_oStopwatch.SetTextureSection( pStopwatch );

	m_oClockRoot.SetDimensions( m_oStopwatch.GetWidth(), m_oStopwatch.GetHeight() );
	m_oClockRoot.AddChildTail( m_oStopwatch );

	m_oWood.AddChildTail( m_oTicker );
	m_oBackground.AddChildTail( m_oWood );
	m_oBackground.AddChildTail( m_oName );

	if ( a_eFlags & FLAGS_STOPWATCH )
	{
		m_oBackground.AddChildTail( m_oClockRoot );
		m_oStopwatch.AddChildTail( m_oClockRed );
		m_oStopwatch.AddChildTail( m_oHide );
		m_oWood.SetWidth( m_oWood.GetWidth() * 1.2f );
	}
	else
	{
		m_oBackground.AddChildTail( m_oHide );
	}

	m_oBackground.SetDimensions( m_oWood.GetWidth(), m_oWood.GetHeight() );
	AddChildTail( m_oBackground );

	Layout( a_eFlags );
	SetDimensions( m_oBackground.GetWidth(), m_oBackground.GetHeight() );

	if ( a_eFlags & FLAGS_STOPWATCH )
		SetupStopwatch();

	// Fit the ticker into the plank
	T2GUITransform oScale;
	oScale.Set( TVector2( 1.0f, 0.0f ), TVector2( 0.0f, fWoodH / ( fTickerHeight * 1.2f ) ), TVector2::VEC_ZERO );
	m_oTicker.GetTransform().PreMultiply( oScale );

	// NOTE: the original looks the font up without using it
	T2GUIFontManager::FindFont( "Rekord18" );

	return TTRUE;
}
