#pragma once
#include "AGUI2ScoreTicker.h"
#include "AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>

//-----------------------------------------------------------------------------
// Purpose: Score counter made of up to three AGUI2ScoreTickers, so that it can
// show integers ("123"), decimals ("12.34") and times ("1:23:45").
// Used by the scorebook (AGamesPlayedDisplayState).
// vtable 0073a850.
//-----------------------------------------------------------------------------
class AGUI2ScoreTickerAdvanced
    : public Toshi::T2GUIElement
{
public:
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_INTEGER = 0, // one ticker
		TYPE_DECIMAL = 1, // ticker "." ticker
		TYPE_TIME    = 2, // minutes ":" seconds ":" hundredths
		TYPE_NONE    = 3,
	};

	static constexpr TINT MAX_TICKERS    = 3;
	static constexpr TINT MAX_SEPARATORS = 2;

public:
	AGUI2ScoreTickerAdvanced();
	~AGUI2ScoreTickerAdvanced();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE; // +0x38

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// The Create methods destroy the current tickers first. a_szWindowTexSec and the
	// digit offsets are passed to AGUI2ScoreTicker::Create.
	TBOOL CreateInteger( TINT a_iNumDigits, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY );
	TBOOL CreateDecimal( TINT a_iNumDigits, TINT a_iNumDecimals, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY );
	TBOOL CreateTime( TINT a_iNumMinuteDigits, TINT a_iNumSecondDigits, TINT a_iNumHundredthDigits, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY );

	// Removes and destroys the tickers
	void Destroy();

	// The height the tickers are scaled to and the translation, applied by the Create methods
	// $Barnyard: FUNCTION 0064bf50
	void SetLayout( TFLOAT a_fHeight, TFLOAT a_fOffsetX, TFLOAT a_fOffsetY )
	{
		m_fHeight  = a_fHeight;
		m_fOffsetX = a_fOffsetX;
		m_fOffsetY = a_fOffsetY;
	}

	void  SetScore( TFLOAT a_fScore );
	void  SetSpinning( TBOOL a_bSpinning );
	TBOOL IsSpinning() const;
	void  ShowFinalScore();
	void  Reset();

	TFLOAT GetScore() const { return m_fScore; }
	TYPE   GetType() const { return m_eType; }

private:
	TBOOL            m_bHideWindows;                  // 0x54, hides the frames of the tickers
	TFLOAT           m_fScore;                        // 0x58
	TFLOAT           m_fHeight;                       // 0x5c
	TFLOAT           m_fOffsetX;                      // 0x60
	TFLOAT           m_fOffsetY;                      // 0x64
	TYPE             m_eType;                         // 0x68
	AGUI2ScoreTicker m_aTickers[ MAX_TICKERS ];       // 0x6c
	AGUI2TextBox     m_aSeparators[ MAX_SEPARATORS ]; // 0x360
	TINT             m_aNumDigits[ MAX_TICKERS ];     // 0x450
};

TSTATICASSERT( sizeof( AGUI2ScoreTickerAdvanced ) == 0x45c );
