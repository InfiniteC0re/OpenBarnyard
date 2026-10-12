#pragma once
#include "GUI/AGUI2ScoreTicker.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIPolygon.h>

//-----------------------------------------------------------------------------
// Player panel of the minigames: the player's hide (portrait) on a wooden
// plank with a score ticker and a name, optionally inside a stopwatch whose
// clock runs down (darts). Plain T2GUIElement (no TClass), vtable 0073a680.
// Name from the Wii build (AGUI2Portrait), size 0x5c8.
//-----------------------------------------------------------------------------
class AGUI2Portrait
    : public Toshi::T2GUIElement
{
public:
	// Flags of Create
	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE      = 0,
		FLAGS_LEFT      = BITFLAG( 0 ), // the panel is on the left side of the screen
		FLAGS_RIGHT     = BITFLAG( 1 ), // the panel is on the right side of the screen
		FLAGS_STOPWATCH = BITFLAG( 3 ), // the hide is shown inside a stopwatch with a clock
		FLAGS_2_DIGITS  = BITFLAG( 4 ), // the score ticker has 2 digits (3 by default)
		FLAGS_UNK5      = BITFLAG( 5 ), // passed by Chasing Chicks and Darts, no effect
		FLAGS_4_DIGITS  = BITFLAG( 6 ), // the score ticker has 4 digits
		FLAGS_NO_NAME   = BITFLAG( 7 ), // hides the name
	};

	static constexpr TFLOAT CLOCK_WARNING_RATIO  = 0.2f;     // 007798d8
	static constexpr TFLOAT CLOCK_CRITICAL_RATIO = 0.1f;     // 007798dc
	static constexpr TFLOAT STOPWATCH_SCALE      = 0.75f;    // 007798e0
	static constexpr TFLOAT TICK_ROTATION        = 0.08727f; // 007798e8 (007798e4 is the negative one)
	static constexpr TFLOAT TIME_UP_SHAKE_TIME   = 1.0f;     // 007798ec

public:
	AGUI2Portrait();
	~AGUI2Portrait();

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void   Tick( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual TFLOAT GetWidth() OVERRIDE;
	virtual TFLOAT GetHeight() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Creates the panel with the a_szHideTexture texture section as the portrait.
	// a_pUnused isn't used.
	TBOOL Create( const TCHAR* a_szHideTexture, FLAGS a_eFlags, Toshi::T2GUIElement* a_pUnused );

	// Detaches all the elements
	void Destroy();

	// Spins the score ticker to a_iScore
	void SetScore( TINT a_iScore );

	void SetName( const TWCHAR* a_wszName );
	void SetNameColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue );
	void SetNameAlpha( TFLOAT a_fAlpha );

	// Stopwatch only
	void SetClockDuration( TFLOAT a_fDuration );
	void SetClockTime( TFLOAT a_fTime );
	void SetClockVisible( TBOOL a_bVisible );
	void SetClockEnabled( TBOOL a_bEnabled );
	void ResetClock( TBOOL a_bResetScore );
	void StartClockFade( TFLOAT a_fFrom, TFLOAT a_fTo, TFLOAT a_fDuration );

private:
	void Layout( FLAGS a_eFlags );
	void SetupStopwatch();
	void CreateClock( TFLOAT a_fWidth, TFLOAT a_fHeight );

public:
	AGUI2ScoreTicker      m_oTicker;        // 0x054
	Toshi::T2GUIRectangle m_oBackground;    // 0x150, invisible, parent of the rest
	Toshi::T2GUIRectangle m_oWood;          // 0x1c8, "Score_Wood"
	Toshi::T2GUIRectangle m_oHide;          // 0x240, the portrait
	Toshi::T2GUIElement   m_oClockRoot;     // 0x2b8
	Toshi::T2GUIRectangle m_oStopwatch;     // 0x30c, "stopwatch"
	Toshi::T2GUIRectangle m_oClockRed;      // 0x384, "clockred"
	AGUI2TextBox          m_oName;          // 0x3fc
	Toshi::T2GUIPolygon   m_oClockPolygon1; // 0x474, second half of the clock
	Toshi::T2GUIPolygon   m_oClockPolygon2; // 0x4dc, first half of the clock
	TFLOAT                m_fClockWidth;    // 0x544
	TFLOAT                m_fClockHeight;   // 0x548
	TFLOAT                m_fClockDuration; // 0x54c
	TFLOAT                m_afUnk1[ 4 ];    // 0x550, [ 3 ] is the width of the name
	TUINT8                PADDING1[ 4 ];    // 0x560
	TWCHAR                m_wszName[ 16 ];  // 0x564
	TINT                  m_iNumDigits;     // 0x584
	FLAGS                 m_eFlags;         // 0x588
	TINT                  m_iUnk2;          // 0x58c, -999
	TINT                  m_iUnk3;          // 0x590
	TINT                  m_iClockSound;    // 0x594, -1 = none
	TINT                  m_iClockCue;      // 0x598
	TFLOAT                m_fClockRatio;    // 0x59c, time left / duration
	TFLOAT                m_fTickTimer;     // 0x5a0
	TFLOAT                m_fTimeUpTimer;   // 0x5a4
	TFLOAT                m_fTickRotation;  // 0x5a8
	TFLOAT                m_fTickInterval;  // 0x5ac
	TFLOAT                m_fFadeFrom;      // 0x5b0
	TFLOAT                m_fFadeTo;        // 0x5b4
	TFLOAT                m_fFadeDuration;  // 0x5b8
	TFLOAT                m_fFadeTime;      // 0x5bc
	TBOOL                 m_bFading;        // 0x5c0
	TUINT8                PADDING2[ 3 ];    // 0x5c1
	TBOOL                 m_bClockEnabled;  // 0x5c4
};

TSTATICASSERT( sizeof( AGUI2Portrait ) == 0x5c8 );
