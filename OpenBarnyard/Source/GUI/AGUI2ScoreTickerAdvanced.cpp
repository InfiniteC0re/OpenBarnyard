#include "pch.h"
#include "AGUI2ScoreTickerAdvanced.h"

#include <GUI/T2GUIFontManager.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0064b880
AGUI2ScoreTickerAdvanced::AGUI2ScoreTickerAdvanced()
{
	m_fScore          = 0.0f;
	m_fOffsetX        = 0.0f;
	m_fOffsetY        = 0.0f;
	m_aNumDigits[ 0 ] = 0;
	m_aNumDigits[ 1 ] = 0;
	m_aNumDigits[ 2 ] = 0;
	m_eType           = TYPE_NONE;
	m_fHeight         = 15.0f;
	m_bHideWindows    = TTRUE;
}

// $Barnyard: FUNCTION 0064c260
// $Barnyard: FUNCTION 0064c2f0
AGUI2ScoreTickerAdvanced::~AGUI2ScoreTickerAdvanced()
{
	Destroy();
}

// $Barnyard: FUNCTION 0064c070
void AGUI2ScoreTickerAdvanced::SetAlpha( TFLOAT a_fAlpha )
{
	// NOTE: the original ignores a_fAlpha
	switch ( m_eType )
	{
		case TYPE_INTEGER:
			m_aTickers[ 0 ].SetAlpha( 0.0f );
			break;
		case TYPE_DECIMAL:
			m_aTickers[ 0 ].SetAlpha( 0.0f );
			m_aTickers[ 1 ].SetAlpha( 0.0f );
			break;
		case TYPE_TIME:
			m_aTickers[ 0 ].SetAlpha( 0.0f );
			m_aTickers[ 1 ].SetAlpha( 0.0f );
			m_aTickers[ 2 ].SetAlpha( 0.0f );
			break;
	}
}

// $Barnyard: FUNCTION 0064b930
void AGUI2ScoreTickerAdvanced::Destroy()
{
	switch ( m_eType )
	{
		case TYPE_INTEGER:
			m_aTickers[ 0 ].RemoveSelf();
			m_aTickers[ 0 ].Destroy();
			break;
		case TYPE_DECIMAL:
			m_aTickers[ 1 ].RemoveSelf();
			m_aSeparators[ 0 ].RemoveSelf();
			m_aTickers[ 0 ].RemoveSelf();
			m_aTickers[ 1 ].Destroy();
			m_aTickers[ 0 ].Destroy();
			break;
		case TYPE_TIME:
			m_aTickers[ 2 ].RemoveSelf();
			m_aSeparators[ 1 ].RemoveSelf();
			m_aTickers[ 1 ].RemoveSelf();
			m_aSeparators[ 0 ].RemoveSelf();
			m_aTickers[ 0 ].RemoveSelf();
			m_aTickers[ 2 ].Destroy();
			m_aTickers[ 1 ].Destroy();
			m_aTickers[ 0 ].Destroy();
			break;
	}

	m_fScore          = 0.0f;
	m_aNumDigits[ 0 ] = 0;
	m_aNumDigits[ 1 ] = 0;
	m_aNumDigits[ 2 ] = 0;
	m_eType           = TYPE_NONE;
}

// $Barnyard: FUNCTION 0064ba20
TBOOL AGUI2ScoreTickerAdvanced::CreateInteger( TINT a_iNumDigits, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY )
{
	Destroy();

	m_oTransform      = T2GUITransform();
	m_eType           = TYPE_INTEGER;
	m_aNumDigits[ 0 ] = a_iNumDigits;

	m_aTickers[ 0 ].Create( a_iNumDigits, a_szWindowTexSec, a_fDigitOffsetX, a_fDigitOffsetY );
	m_aTickers[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	AddChildTail( m_aTickers[ 0 ] );

	SetDimensions( m_aTickers[ 0 ].GetWidth(), m_aTickers[ 0 ].GetHeight() );
	m_aTickers[ 0 ].m_oWindow.SetAlpha( m_bHideWindows ? 0.0f : 1.0f );

	// Scale it to the wanted height
	TFLOAT fScale = m_fHeight / m_aTickers[ 0 ].GetHeight();
	m_oTransform.Scale( fScale, fScale );
	m_oTransform.SetTranslation( m_fOffsetX, m_fOffsetY );

	return TTRUE;
}

// $Barnyard: FUNCTION 0064baf0
TBOOL AGUI2ScoreTickerAdvanced::CreateDecimal( TINT a_iNumDigits, TINT a_iNumDecimals, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY )
{
	Destroy();

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	m_oTransform      = T2GUITransform();
	m_eType           = TYPE_DECIMAL;
	m_aNumDigits[ 0 ] = a_iNumDigits;

	m_aTickers[ 0 ].Create( a_iNumDigits, a_szWindowTexSec, a_fDigitOffsetX, a_fDigitOffsetY );
	m_aTickers[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	// The decimal point, right of the integer part
	m_aSeparators[ 0 ].Create( pFont, 5.0f );
	m_aSeparators[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSeparators[ 0 ].SetTextAndFitWidth( L"." );
	m_aTickers[ 0 ].AddChildTail( m_aSeparators[ 0 ] );

	// The decimals, right of the point
	m_aNumDigits[ 1 ] = a_iNumDecimals;
	m_aTickers[ 1 ].Create( a_iNumDecimals, a_szWindowTexSec, a_fDigitOffsetX, a_fDigitOffsetY );
	m_aTickers[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSeparators[ 0 ].AddChildTail( m_aTickers[ 1 ] );

	AddChildTail( m_aTickers[ 0 ] );

	TFLOAT fHeight = m_aTickers[ 0 ].GetHeight();
	TFLOAT fWidth  = m_aSeparators[ 0 ].GetWidth();
	fWidth         = m_aTickers[ 0 ].GetWidth() + fWidth;
	fWidth         = m_aTickers[ 1 ].GetWidth() + fWidth;
	SetDimensions( fWidth, fHeight );

	if ( m_bHideWindows )
	{
		m_aTickers[ 0 ].m_oWindow.SetAlpha( 0.0f );
		m_aTickers[ 1 ].m_oWindow.SetAlpha( 0.0f );
	}
	else
	{
		m_aTickers[ 0 ].m_oWindow.SetAlpha( 1.0f );
		m_aTickers[ 1 ].m_oWindow.SetAlpha( 1.0f );
	}

	// Scale it to the wanted height
	TFLOAT fScale = m_fHeight / m_aTickers[ 0 ].GetHeight();
	m_oTransform.Scale( fScale, fScale );
	m_oTransform.SetTranslation( m_fOffsetX, m_fOffsetY );

	return TTRUE;
}

// $Barnyard: FUNCTION 0064bca0
TBOOL AGUI2ScoreTickerAdvanced::CreateTime( TINT a_iNumMinuteDigits, TINT a_iNumSecondDigits, TINT a_iNumHundredthDigits, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY )
{
	Destroy();

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	m_oTransform      = T2GUITransform();
	m_eType           = TYPE_TIME;
	m_aNumDigits[ 0 ] = a_iNumMinuteDigits;

	// Minutes
	m_aTickers[ 0 ].Create( a_iNumMinuteDigits, a_szWindowTexSec, a_fDigitOffsetX, a_fDigitOffsetY );
	m_aTickers[ 0 ].SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );

	m_aSeparators[ 0 ].Create( pFont, 5.0f );
	m_aSeparators[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSeparators[ 0 ].SetTextAndFitWidth( L":" );
	m_aTickers[ 0 ].AddChildTail( m_aSeparators[ 0 ] );

	// Seconds
	m_aNumDigits[ 1 ] = a_iNumSecondDigits;
	m_aTickers[ 1 ].Create( a_iNumSecondDigits, a_szWindowTexSec, a_fDigitOffsetX, a_fDigitOffsetY );
	m_aTickers[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSeparators[ 0 ].AddChildTail( m_aTickers[ 1 ] );

	m_aSeparators[ 1 ].Create( pFont, 5.0f );
	m_aSeparators[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSeparators[ 1 ].SetTextAndFitWidth( L":" );
	m_aTickers[ 1 ].AddChildTail( m_aSeparators[ 1 ] );

	// Hundredths
	m_aNumDigits[ 2 ] = a_iNumHundredthDigits;
	m_aTickers[ 2 ].Create( a_iNumHundredthDigits, a_szWindowTexSec, a_fDigitOffsetX, a_fDigitOffsetY );
	m_aTickers[ 2 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );
	m_aSeparators[ 1 ].AddChildTail( m_aTickers[ 2 ] );

	AddChildTail( m_aTickers[ 0 ] );

	TFLOAT fHeight = m_aTickers[ 0 ].GetHeight();
	TFLOAT fWidth  = m_aSeparators[ 1 ].GetWidth();
	fWidth         = m_aTickers[ 0 ].GetWidth() + fWidth;
	fWidth         = m_aTickers[ 1 ].GetWidth() + fWidth;
	fWidth         = m_aTickers[ 2 ].GetWidth() + fWidth;
	fWidth         = m_aSeparators[ 0 ].GetWidth() + fWidth;
	SetDimensions( fWidth, fHeight );

	if ( m_bHideWindows )
	{
		m_aTickers[ 0 ].m_oWindow.SetAlpha( 0.0f );
		m_aTickers[ 1 ].m_oWindow.SetAlpha( 0.0f );
		m_aTickers[ 2 ].m_oWindow.SetAlpha( 0.0f );
	}
	else
	{
		m_aTickers[ 0 ].m_oWindow.SetAlpha( 1.0f );
		m_aTickers[ 1 ].m_oWindow.SetAlpha( 1.0f );
		m_aTickers[ 2 ].m_oWindow.SetAlpha( 1.0f );
	}

	// Scale it to the wanted height
	TFLOAT fScale = m_fHeight / m_aTickers[ 0 ].GetHeight();
	m_oTransform.Scale( fScale, fScale );
	m_oTransform.SetTranslation( m_fOffsetX, m_fOffsetY );

	return TTRUE;
}

// $Barnyard: FUNCTION 0064bf70
void AGUI2ScoreTickerAdvanced::SetScore( TFLOAT a_fScore )
{
	m_fScore = a_fScore;

	switch ( m_eType )
	{
		case TYPE_INTEGER:
			m_aTickers[ 0 ].SetScore( TINT( a_fScore ) );
			break;
		case TYPE_DECIMAL:
		{
			TINT iInteger  = TMath::FloorToInt( a_fScore );
			TINT iDecimals = TINT( TMath::Pow( 10.0f, TFLOAT( m_aNumDigits[ 1 ] ) ) * ( a_fScore - TFLOAT( iInteger ) ) );

			m_aTickers[ 0 ].SetScore( iInteger );
			m_aTickers[ 1 ].SetScore( iDecimals );
			break;
		}
		case TYPE_TIME:
		{
			TINT iSeconds = TMath::FloorToInt( a_fScore ) % 60;

			m_aTickers[ 0 ].SetScore( ( TMath::FloorToInt( a_fScore ) - iSeconds ) / 60 );
			m_aTickers[ 1 ].SetScore( iSeconds );
			m_aTickers[ 2 ].SetScore( TINT( fmod( a_fScore * 100.0f, 100.0 ) ) );
			break;
		}
	}
}

// $Barnyard: FUNCTION 0064c0f0
void AGUI2ScoreTickerAdvanced::SetSpinning( TBOOL a_bSpinning )
{
	switch ( m_eType )
	{
		case TYPE_INTEGER:
			m_aTickers[ 0 ].SetSpinning( a_bSpinning );
			break;
		case TYPE_DECIMAL:
			m_aTickers[ 0 ].SetSpinning( a_bSpinning );
			m_aTickers[ 1 ].SetSpinning( a_bSpinning );
			break;
		case TYPE_TIME:
			m_aTickers[ 0 ].SetSpinning( a_bSpinning );
			m_aTickers[ 1 ].SetSpinning( a_bSpinning );
			m_aTickers[ 2 ].SetSpinning( a_bSpinning );
			break;
	}
}

// $Barnyard: FUNCTION 0064c160
TBOOL AGUI2ScoreTickerAdvanced::IsSpinning() const
{
	switch ( m_eType )
	{
		case TYPE_INTEGER:
			return m_aTickers[ 0 ].IsSpinning();
		case TYPE_DECIMAL:
			return m_aTickers[ 0 ].IsSpinning() || m_aTickers[ 1 ].IsSpinning();
		case TYPE_TIME:
			return m_aTickers[ 0 ].IsSpinning() || m_aTickers[ 1 ].IsSpinning() || m_aTickers[ 2 ].IsSpinning();
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0064c1c0
void AGUI2ScoreTickerAdvanced::ShowFinalScore()
{
	switch ( m_eType )
	{
		case TYPE_INTEGER:
			m_aTickers[ 0 ].ShowFinalScore();
			break;
		case TYPE_DECIMAL:
			m_aTickers[ 0 ].ShowFinalScore();
			m_aTickers[ 1 ].ShowFinalScore();
			break;
		case TYPE_TIME:
			m_aTickers[ 0 ].ShowFinalScore();
			m_aTickers[ 1 ].ShowFinalScore();
			m_aTickers[ 2 ].ShowFinalScore();
			break;
	}
}

// $Barnyard: FUNCTION 0064c210
void AGUI2ScoreTickerAdvanced::Reset()
{
	switch ( m_eType )
	{
		case TYPE_INTEGER:
			m_aTickers[ 0 ].Reset();
			break;
		case TYPE_DECIMAL:
			m_aTickers[ 0 ].Reset();
			m_aTickers[ 1 ].Reset();
			break;
		case TYPE_TIME:
			m_aTickers[ 0 ].Reset();
			m_aTickers[ 1 ].Reset();
			m_aTickers[ 2 ].Reset();
			break;
	}
}
