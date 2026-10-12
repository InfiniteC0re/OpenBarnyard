#include "pch.h"
#include "AGUI2ScoreTicker.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"

#include <GUI/T2GUITextureSectionManager.h>
#include <Render/TRender.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0064a130
AGUI2ScoreTicker::AGUI2ScoreTicker()
{
	m_bSpinning     = TFALSE;
	m_iScore        = 0;
	m_iNumDigits    = 0;
	m_pDigitStates  = TNULL;
	m_pDigitTargets = TNULL;
	m_pDigitValues  = TNULL;
	m_pDrums        = TNULL;
	m_pNumbers      = TNULL;
	m_pGlints       = TNULL;
	m_pClickTimers  = TNULL;
	m_fTimer        = 0.0f;
	m_iCurrentDigit = 0;
}

// $Barnyard: FUNCTION 0064a550
// $Barnyard: FUNCTION 0064b730
AGUI2ScoreTicker::~AGUI2ScoreTicker()
{
	Destroy();
}

// $Barnyard: FUNCTION 0064a1b0
void AGUI2ScoreTicker::Destroy()
{
	for ( TINT i = 0; i < m_iNumDigits; i++ )
	{
		m_pDrums[ i ].RemoveSelf();
		m_pNumbers[ i ].RemoveSelf();
		m_pGlints[ i ].RemoveSelf();
	}

	delete[] m_pDigitStates;
	m_pDigitStates = TNULL;

	delete[] m_pDigitTargets;
	m_pDigitTargets = TNULL;

	delete[] m_pDigitValues;
	m_pDigitValues = TNULL;

	delete[] m_pDrums;
	m_pDrums = TNULL;

	delete[] m_pNumbers;
	m_pNumbers = TNULL;

	delete[] m_pGlints;
	m_pGlints = TNULL;

	delete[] m_pClickTimers;
	m_pClickTimers = TNULL;

	m_oWindow.RemoveSelf();
	m_fTimer        = 0.0f;
	m_iCurrentDigit = 0;
	m_iNumDigits    = 0;
	m_bSpinning     = TFALSE;
	m_iScore        = 0;
}

// $Barnyard: FUNCTION 0064a5b0
TBOOL AGUI2ScoreTicker::Create( TINT a_iNumDigits, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY )
{
	if ( m_pDigitStates || m_pDigitTargets || m_pDrums || m_pDigitValues )
		return TFALSE;

	m_iNumDigits    = a_iNumDigits;
	m_pDigitStates  = new DIGITSTATE[ m_iNumDigits ];
	m_pDigitTargets = new TFLOAT[ m_iNumDigits ];
	m_pDigitValues  = new TINT[ m_iNumDigits ];
	m_pClickTimers  = new TFLOAT[ m_iNumDigits ];
	m_pDrums        = new T2GUIRectangle[ m_iNumDigits ];
	m_pNumbers      = new T2GUIRectangle[ m_iNumDigits ];
	m_pGlints       = new T2GUIRectangle[ m_iNumDigits ];

	T2GUITextureSection* pWindow = T2GUITextureSectionManager::GetTextureSection( a_szWindowTexSec ? a_szWindowTexSec : "Wheel_Window" );
	m_oWindow.SetTextureSection( pWindow );

	TFLOAT fWindowWidth     = TMath::Abs( TFLOAT( pWindow->GetPoint2X() - pWindow->GetPoint1X() ) );
	TFLOAT fInvWindowHeight = 1.0f / TMath::Abs( TFLOAT( pWindow->GetPoint2Y() - pWindow->GetPoint1Y() ) );
	TFLOAT fDefaultOffsetY  = fInvWindowHeight * 11.0f;

	T2GUITextureSection* pDrum       = T2GUITextureSectionManager::GetTextureSection( "Score_Drum" );
	TFLOAT               fDrumWidth  = TFLOAT( pDrum->GetPoint2X() - pDrum->GetPoint1X() );
	TFLOAT               fDrumHeight = TFLOAT( pDrum->GetPoint2Y() - pDrum->GetPoint1Y() );

	T2GUITextureSection* pGlint   = T2GUITextureSectionManager::GetTextureSection( "Score_Drum_Glint" );
	T2GUITextureSection* pNumbers = T2GUITextureSectionManager::GetTextureSection( "Wheel_Numbers" );

	// The numbers scroll vertically, so their texture has to wrap
	pNumbers->GetMaterial()->GetTextureResource()->SetAddressing( ADDRESSINGMODE_WRAP );

	TFLOAT fInvWindowWidth = 1.0f / fWindowWidth;
	m_oWindow.SetDimensions(
	    TMath::Abs( fDrumWidth ) * m_iNumDigits / ( fInvWindowWidth * 75.0f ) * ms_fWindowScaleX,
	    TMath::Abs( fDrumHeight ) / ( fInvWindowHeight * 34.0f ) * ms_fWindowScaleY
	);

	TFLOAT fOffsetX = 4.0f;

	TFLOAT fDigitOffsetX = ( a_fDigitOffsetX >= 0.0f ) ? a_fDigitOffsetX : m_oWindow.GetWidth() * ( fInvWindowWidth * 26.0f );
	TFLOAT fDigitOffsetY = ( a_fDigitOffsetY >= 0.0f ) ? a_fDigitOffsetY : m_oWindow.GetHeight() * fDefaultOffsetY;

	for ( TINT i = 0; i < m_iNumDigits; i++ )
	{
		m_pDigitStates[ i ] = DIGITSTATE_WAITING;

		m_pDrums[ i ].SetTextureSection( *pDrum );
		m_pDrums[ i ].SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
		m_pDrums[ i ].SetTransform( -fOffsetX - fDigitOffsetX, fDigitOffsetY );

		m_pGlints[ i ].SetTextureSection( *pGlint );

		m_pNumbers[ i ].SetTextureSection( *pNumbers );
		m_pNumbers[ i ].SetColour( 0xff000000 );
		m_pNumbers[ i ].SetHeight( m_pDrums[ i ].GetHeight() * 0.8f );
		m_pNumbers[ i ].SetUVs( TVector2( 0.0f, 0.0f ), TVector2( 1.0f, 0.0f ), TVector2( 1.0f, ms_fDigitHeight ), TVector2( 0.0f, ms_fDigitHeight ) );

		m_oWindow.AddChildTail( m_pDrums[ i ] );
		m_pDrums[ i ].AddChildTail( m_pNumbers[ i ] );
		m_pDrums[ i ].AddChildTail( m_pGlints[ i ] );

		m_pClickTimers[ i ] = 0.0f;
		fOffsetX += m_pDrums[ i ].GetWidth();
		m_pDigitTargets[ i ] = 0.0f;
		m_pDigitValues[ i ]  = 0;
	}

	AddChildTail( m_oWindow );
	SetDimensions( m_oWindow.GetWidth(), m_oWindow.GetHeight() );
	m_bSpinning = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 0064a460
void AGUI2ScoreTicker::Reset()
{
	m_bSpinning     = TFALSE;
	m_iScore        = 0;
	m_fTimer        = 0.0f;
	m_iCurrentDigit = 0;

	for ( TINT i = 0; i < m_iNumDigits; i++ )
	{
		m_pDigitStates[ i ]  = DIGITSTATE_IDLE;
		m_pDigitTargets[ i ] = 0.0f;
		m_pDigitValues[ i ]  = 0;
		m_pNumbers[ i ].Hide();
		m_pNumbers[ i ].SetUVs( TVector2( 0.0f, 0.0f ), TVector2( 1.0f, 0.0f ), TVector2( 1.0f, ms_fDigitHeight ), TVector2( 0.0f, ms_fDigitHeight ) );
		m_pClickTimers[ i ] = 0.0f;
	}
}

// NOTE: inlined in the original
void AGUI2ScoreTicker::ResetDigit( TINT a_iDigit )
{
	m_pNumbers[ a_iDigit ].Hide();
	m_pDigitStates[ a_iDigit ]  = DIGITSTATE_IDLE;
	m_pDigitTargets[ a_iDigit ] = 0.0f;
	m_pNumbers[ a_iDigit ].SetUVs( TVector2( 0.0f, 0.0f ), TVector2( 1.0f, 0.0f ), TVector2( 1.0f, ms_fDigitHeight ), TVector2( 0.0f, ms_fDigitHeight ) );
	m_pDigitValues[ a_iDigit ] = 0;
}

// NOTE: inlined in the original
void AGUI2ScoreTicker::HideUnusedDigits( TINT a_iScore )
{
	if ( a_iScore != 0 )
	{
		TINT iFirstUnused;

		if ( m_iScore < 10 )
			iFirstUnused = 1;
		else if ( m_iScore < 100 )
			iFirstUnused = 2;
		else if ( m_iScore < 1000 )
			iFirstUnused = 3;
		else
			return;

		for ( TINT i = iFirstUnused; i < m_iNumDigits; i++ )
			ResetDigit( i );
	}
	else
	{
		// Zero shows a single digit
		m_pNumbers[ 0 ].Show();
		m_pDigitStates[ 0 ]  = DIGITSTATE_IDLE;
		m_pDigitTargets[ 0 ] = 0.0f;
		m_pNumbers[ 0 ].SetUVs( TVector2( 0.0f, 0.0f ), TVector2( 1.0f, 0.0f ), TVector2( 1.0f, ms_fDigitHeight ), TVector2( 0.0f, ms_fDigitHeight ) );
		m_pDigitValues[ 0 ] = 0;

		for ( TINT i = 1; i < m_iNumDigits; i++ )
			ResetDigit( i );
	}
}

// $Barnyard: FUNCTION 0064aac0
void AGUI2ScoreTicker::SetScore( TINT a_iScore )
{
	if ( a_iScore == m_iScore )
	{
		HideUnusedDigits( a_iScore );
		return;
	}

	// Negative scores are red
	TUINT32 uiColour = ( a_iScore >= 0 ) ? 0xff000000 : 0xffff0000;

	for ( TINT i = 0; i < m_iNumDigits; i++ )
		m_pNumbers[ i ].SetColour( uiColour );

	TINT iMaxScore = TINT( TMath::Pow( 10.0f, TFLOAT( m_iNumDigits ) ) ) - 1;
	TINT iScore    = ( a_iScore < 0 ) ? -a_iScore : a_iScore;
	m_iScore       = ( iScore <= iMaxScore ) ? iScore : iMaxScore;

	TINT iValue = m_iScore;

	for ( TINT i = 0; i < m_iNumDigits; i++ )
	{
		TINT iDigit = iValue % 10;
		iValue /= 10;

		m_pDigitTargets[ i ] = TFLOAT( iDigit ) * ms_fDigitHeight;
		m_pNumbers[ i ].Show();

		if ( iDigit == m_pDigitValues[ i ] )
		{
			// The digit doesn't change
			m_pDigitStates[ i ] = DIGITSTATE_IDLE;

			TVector2 vUV1, vUV2, vUV3, vUV4;
			m_pNumbers[ i ].GetUVs( vUV1, vUV2, vUV3, vUV4 );
			vUV1.y = m_pDigitTargets[ i ];
			vUV2.y = m_pDigitTargets[ i ];
			vUV3.y = m_pDigitTargets[ i ] + ms_fDigitHeight;
			vUV4.y = vUV3.y;
			m_pNumbers[ i ].SetUVs( vUV1, vUV2, vUV3, vUV4 );

			m_pClickTimers[ i ] = 0.0f;
		}
		else if ( i != 0 )
		{
			m_pDigitStates[ i ] = DIGITSTATE_WAITING;
		}
		else
		{
			m_pDigitStates[ 0 ] = DIGITSTATE_SPINNING;
		}

		m_pDigitValues[ i ] = iDigit;
		m_iCurrentDigit     = 0;
	}

	HideUnusedDigits( a_iScore );
}

// $Barnyard: FUNCTION 0064a340
void AGUI2ScoreTicker::ShowFinalScore()
{
	if ( m_bSpinning == TTRUE || !m_pNumbers[ 0 ].IsVisible() )
	{
		for ( TINT i = 0; i < m_iNumDigits; i++ )
		{
			if ( m_iScore < 10 && i > 0 )
			{
				m_pNumbers[ i ].Hide();
			}
			else if ( m_iScore < 100 && i > 1 )
			{
				// NOTE: the original always hides the third digit here
				m_pNumbers[ 2 ].Hide();
			}
			else
			{
				m_pNumbers[ i ].Show();
			}

			TVector2 vUV1, vUV2, vUV3, vUV4;
			m_pNumbers[ i ].GetUVs( vUV1, vUV2, vUV3, vUV4 );
			vUV2.y = m_pDigitTargets[ i ];
			vUV1.y = vUV2.y;
			vUV4.y = ms_fDigitHeight + m_pDigitTargets[ i ];
			vUV3.y = vUV4.y;
			m_pNumbers[ i ].SetUVs( vUV1, vUV2, vUV3, vUV4 );
		}
	}
}

// $Barnyard: FUNCTION 0064b490
void AGUI2ScoreTicker::UpdateDigit( TINT a_iDigit, TFLOAT a_fDeltaTime )
{
	if ( !m_pNumbers[ a_iDigit ].IsVisible() || a_iDigit < 0 || a_iDigit >= m_iNumDigits )
		return;

	TVector2 aUVs[ 4 ];
	m_pNumbers[ a_iDigit ].GetUVs( aUVs[ 0 ], aUVs[ 1 ], aUVs[ 2 ], aUVs[ 3 ] );

	TFLOAT fTarget = m_pDigitTargets[ a_iDigit ];

	if ( m_pDigitStates[ a_iDigit ] == DIGITSTATE_SPINNING )
	{
		for ( TINT i = 0; i < 2; i++ )
		{
			TFLOAT fStep = ms_fSpinSpeed * a_fDeltaTime;
			aUVs[ i ].y += fStep;

			if ( i == 0 )
			{
				// Click every time a digit passes
				m_pClickTimers[ a_iDigit ] += fStep;

				if ( m_pClickTimers[ a_iDigit ] >= ms_fDigitHeight )
				{
					m_pClickTimers[ a_iDigit ] = 0.0f;
					ASoundManager::GetSingleton()->PlayCue( soundbank::UI_SCORE_COUNT );
				}
			}

			aUVs[ i ].y = fmodf( aUVs[ i ].y, 1.0f );
		}

		if ( aUVs[ 0 ].y >= fTarget && TMath::Abs( aUVs[ 0 ].y - fTarget ) <= 0.1f )
		{
			// Reached the target
			m_pDigitStates[ a_iDigit ] = DIGITSTATE_SETTLING;
			aUVs[ 0 ].y                = fTarget;
			aUVs[ 1 ].y                = fTarget;
			m_pClickTimers[ a_iDigit ] = 0.0f;
		}

		aUVs[ 2 ].y = ms_fDigitHeight + aUVs[ 0 ].y;
		aUVs[ 3 ].y = aUVs[ 2 ].y;
		m_pNumbers[ a_iDigit ].SetUVs( aUVs[ 0 ], aUVs[ 1 ], aUVs[ 2 ], aUVs[ 3 ] );
	}
	else if ( m_pDigitStates[ a_iDigit ] == DIGITSTATE_SETTLING )
	{
		m_pClickTimers[ a_iDigit ] += ms_fSpinSpeed * a_fDeltaTime;

		if ( TMath::TWO_PI / ms_fSettleFrequency < m_pClickTimers[ a_iDigit ] )
		{
			// Done wobbling
			aUVs[ 1 ].y                = m_pDigitTargets[ a_iDigit ];
			aUVs[ 0 ].y                = aUVs[ 1 ].y;
			aUVs[ 3 ].y                = ms_fDigitHeight + aUVs[ 1 ].y;
			aUVs[ 2 ].y                = aUVs[ 3 ].y;
			m_pDigitStates[ a_iDigit ] = DIGITSTATE_IDLE;
		}
		else
		{
			// Damped wobble around the target
			TFLOAT fTime = ms_fSettleFrequency * m_pClickTimers[ a_iDigit ];
			TFLOAT fV    = expf( fTime * -0.5f ) * TMath::Sin( fTime ) * 0.05f + m_pDigitTargets[ a_iDigit ];

			if ( fV < 0.0f )
				fV = 1.0f - fV;

			aUVs[ 1 ].y = fmodf( fV, 1.0f );
			aUVs[ 0 ].y = aUVs[ 1 ].y;
			aUVs[ 3 ].y = ms_fDigitHeight + aUVs[ 1 ].y;
			aUVs[ 2 ].y = aUVs[ 3 ].y;
		}

		m_pNumbers[ a_iDigit ].SetUVs( aUVs[ 0 ], aUVs[ 1 ], aUVs[ 2 ], aUVs[ 3 ] );
	}
}

// $Barnyard: FUNCTION 0064b750
void AGUI2ScoreTicker::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bSpinning == TTRUE )
	{
		m_fTimer += a_fDeltaTime;

		if ( m_fTimer >= 0.2f || m_pDigitStates[ m_iCurrentDigit ] == DIGITSTATE_IDLE )
		{
			// Time to start spinning the next digit
			m_fTimer = 0.0f;

			if ( m_iCurrentDigit < m_iNumDigits - 1 )
			{
				TINT iDigit     = m_iCurrentDigit + 1;
				m_iCurrentDigit = iDigit;

				if ( m_pDigitStates[ iDigit ] == DIGITSTATE_IDLE )
				{
					// Skip the digits that don't change
					do
					{
						if ( !m_pNumbers[ iDigit ].IsVisible() || iDigit >= m_iNumDigits - 1 )
							break;

						m_iCurrentDigit = ++iDigit;
					} while ( m_pDigitStates[ iDigit ] == DIGITSTATE_IDLE );
				}
			}

			if ( m_iCurrentDigit < m_iNumDigits && m_pNumbers[ m_iCurrentDigit ].IsVisible() && m_pDigitStates[ m_iCurrentDigit ] == DIGITSTATE_WAITING )
				m_pDigitStates[ m_iCurrentDigit ] = DIGITSTATE_SPINNING;
		}

		TINT iNumIdle = 0;

		for ( TINT i = 0; i < m_iNumDigits; i++ )
		{
			UpdateDigit( i, a_fDeltaTime );

			if ( m_pDigitStates[ i ] == DIGITSTATE_IDLE )
				iNumIdle++;
		}

		if ( iNumIdle == m_iNumDigits )
			m_bSpinning = TFALSE;
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $Barnyard: FUNCTION 0064a2f0
void AGUI2ScoreTicker::SetAlpha( TFLOAT a_fAlpha )
{
	for ( TINT i = 0; i < m_iNumDigits; i++ )
		m_pDrums[ i ].SetAlpha( a_fAlpha );
}
