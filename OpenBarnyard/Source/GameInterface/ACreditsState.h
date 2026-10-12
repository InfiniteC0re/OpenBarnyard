#pragma once
#include "AGameState.h"
#include "GUI/T2GUIRectangle.h"
#include "GUI/AGUI2TextBox.h"

#include <Toshi/TDList.h>

//-----------------------------------------------------------------------------
// The scrolling credits (TClass 007819a0, vtable 0071a6f0), loaded from the
// "Main" properties of Data/credits.trb. Opened from the Bonuses menu and by
// the SS11 ending sequence. Plays the credits voice-over (substream 864 of
// QuestVO), then the credits music, and removes itself once every line has
// scrolled out of the screen.
//-----------------------------------------------------------------------------
class ACreditsState : public AGameState
{
public:
	TDECLARE_CLASS( ACreditsState, AGameState );

	static constexpr TINT MAX_NUM_LINES = 350;

	using LINEFLAGS = TUINT32;
	enum LINEFLAGS_ : LINEFLAGS
	{
		LINEFLAGS_NONE      = 0,
		LINEFLAGS_TITLE     = BITFLAG( 0 ),
		LINEFLAGS_NAME      = BITFLAG( 1 ), // a name that fits the screen
		LINEFLAGS_LONGNAME  = BITFLAG( 2 ), // a name wider than the screen
		LINEFLAGS_IMAGE     = BITFLAG( 3 ),
		LINEFLAGS_LOCALISED = BITFLAG( 4 ), // the text belongs to the locale, it's not freed
	};

	struct Line : public Toshi::TDList<Line>::TNode
	{
		// $Barnyard: FUNCTION 00433480
		Line()
		{
			wszText  = TNULL;
			pElement = TNULL;
			eFlags   = LINEFLAGS_NONE;
		}

		~Line();

		const TWCHAR*        wszText;  // 0x08
		Toshi::T2GUIElement* pElement; // 0x0c
		LINEFLAGS            eFlags;   // 0x10
	};

public:
	// a_bResetFade:  makes the screen opaque (ASSeqFade) once the credits are removed
	// a_bBackground: shows the credits over a black background
	ACreditsState( TBOOL a_bResetFade = TFALSE, TBOOL a_bBackground = TFALSE );
	~ACreditsState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

private:
	// Creates the lines of the "Main" properties of Data/<a_szFileName>.trb
	TBOOL Load( const TCHAR* a_szFileName );

private:
	// Times between two lines, depending on their kinds
	inline static TFLOAT ms_fScrollSpeed = 2.76f;                   // 0x00773470
	inline static TFLOAT ms_fLongDelay   = 1.5f / ms_fScrollSpeed;  // 0x00781994
	inline static TFLOAT ms_fScrollRate  = ms_fScrollSpeed * 30.0f; // 0x00781998, pixels per second
	inline static TFLOAT ms_fShortDelay  = 1.0f / ms_fScrollSpeed;  // 0x0078199c

private:
	Line*                 m_pLines;         // 0xa4
	Toshi::TDList<Line>   m_llFreeLines;    // 0xa8
	Toshi::TDList<Line>   m_llVisibleLines; // 0xb0, scrolling up the screen
	Toshi::TDList<Line>   m_llPendingLines; // 0xb8, waiting for their turn
	Toshi::T2GUIFont*     m_pTitleFont;     // 0xc0
	Toshi::T2GUIFont*     m_pNameFont;      // 0xc4
	TFLOAT                m_fNextLineTime;  // 0xc8
	TINT                  m_iMusicCue;      // 0xcc
	TBOOL                 m_bBackground;    // 0xd0
	Toshi::T2GUIRectangle m_oBackground;    // 0xd4
	TBOOL                 m_bVOStarted;     // 0x14c
	TBOOL                 m_bResetFade;     // 0x14d
};
