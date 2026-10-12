#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Rolling score counter (vtable 0073a7c0): a "Wheel_Window" frame with one
// "Score_Drum" per digit, each one showing a strip of "Wheel_Numbers" that
// spins until it reaches the digit of the score.
// Used by the results table of the minigames (AAnticScoreState::ScoreStat).
//
// Digit states:
//  DIGITSTATE_IDLE:     shows its target digit.
//  DIGITSTATE_WAITING:  waits for the previous digit before spinning.
//  DIGITSTATE_SPINNING: scrolls the numbers until it reaches its target.
//  DIGITSTATE_SETTLING: wobbles around its target before stopping.
//-----------------------------------------------------------------------------
class AGUI2ScoreTicker
    : public Toshi::T2GUIElement
{
public:
	// Calls Destroy() and hides m_oWindow
	friend class AGUI2ScoreTickerAdvanced;

	enum DIGITSTATE : TINT
	{
		DIGITSTATE_IDLE,
		DIGITSTATE_WAITING,
		DIGITSTATE_SPINNING,
		DIGITSTATE_SETTLING,
	};

	// Height of a single digit in the "Wheel_Numbers" texture (V coordinates)
	inline static TFLOAT ms_fDigitHeight = 0.0984375f; // 0x0077990c

	inline static TFLOAT ms_fWindowScaleX    = 1.02f; // 0x00779910
	inline static TFLOAT ms_fWindowScaleY    = 1.01f; // 0x00779914
	inline static TFLOAT ms_fSettleFrequency = 15.0f; // 0x00779918
	inline static TFLOAT ms_fSpinSpeed       = 0.75f; // 0x0077991c

public:
	AGUI2ScoreTicker();
	~AGUI2ScoreTicker();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Creates the frame and the drums. a_szWindowTexSec can be TNULL ("Wheel_Window"),
	// a negative a_fDigitOffsetX/a_fDigitOffsetY means the default offset.
	TBOOL Create( TINT a_iNumDigits, const TCHAR* a_szWindowTexSec, TFLOAT a_fDigitOffsetX, TFLOAT a_fDigitOffsetY );

	// Hides every digit and stops spinning.
	void Reset();

	// Sets the score the digits spin to.
	void SetScore( TINT a_iScore );

	// Shows the final score without spinning.
	void ShowFinalScore();

	// Starts/stops spinning the digits.
	// $Barnyard: FUNCTION 0064a450
	void  SetSpinning( TBOOL a_bSpinning ) { m_bSpinning = a_bSpinning; }
	TBOOL IsSpinning() const { return m_bSpinning; }

	TINT GetScore() const { return m_iScore; }

	// Frees the drums (AHighScoreChart calls it when the chart is cleared)
	void Destroy();

private:
	void UpdateDigit( TINT a_iDigit, TFLOAT a_fDeltaTime );

	// Hides the leading zeros of m_iScore (the original inlines it in SetScore twice)
	void HideUnusedDigits( TINT a_iScore );
	void ResetDigit( TINT a_iDigit );

private:
	TBOOL                  m_bSpinning;     // 0x54
	TINT                   m_iScore;        // 0x58
	TINT                   m_iNumDigits;    // 0x5c
	DIGITSTATE*            m_pDigitStates;  // 0x60
	TFLOAT*                m_pDigitTargets; // 0x64, V coordinate of the target digit
	TINT*                  m_pDigitValues;  // 0x68
	Toshi::T2GUIRectangle  m_oWindow;       // 0x6c
	Toshi::T2GUIRectangle* m_pDrums;        // 0xe4
	Toshi::T2GUIRectangle* m_pNumbers;      // 0xe8
	Toshi::T2GUIRectangle* m_pGlints;       // 0xec
	TFLOAT*                m_pClickTimers;  // 0xf0, time since the last click sound
	TFLOAT                 m_fTimer;        // 0xf4
	TINT                   m_iCurrentDigit; // 0xf8
};

TSTATICASSERT( sizeof( AGUI2ScoreTicker ) == 0xfc );
