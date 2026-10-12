#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2ScoreTicker.h"
#include "MiniGame/AMiniGameState.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Results table of a minigame (TClass 00781d40, vtable 0071b928).
// Added by AAnticGameOverState as its child state. Shows the rows of the
// current player of AMiniGameState::S1 (one ScoreStat each, the total row
// last), then brands a grade (Grade_Star..Grade_F) on the board.
//
// Flow (OnUpdate):
//  STATE_WAIT:         waits half a second.
//  STATE_STATS:        the tickers of the rows spin one after the other.
//  STATE_TOTAL:        waits for the total row, then shows the grade text.
//  STATE_BRAND:        the branding iron moves to the grade and stamps it.
//  STATE_SMOKE:        smoke puffs come out of the brand.
//  STATE_BRAND_RETURN: the iron goes back, the grade fades to its final colour
//                      and the "press OK" text appears.
// OK skips the current row; once the "press OK" text is shown, OK continues:
// with a single human player AAnticHighScoreState replaces this state,
// otherwise it removes itself.
//-----------------------------------------------------------------------------
class AAnticScoreState
    : public AGameState
{
public:
	TDECLARE_CLASS( AAnticScoreState, AGameState );

	enum STATE : TINT
	{
		STATE_WAIT,
		STATE_STATS,
		STATE_TOTAL,
		STATE_BRAND,
		STATE_SMOKE,
		STATE_BRAND_RETURN,
	};

	enum GRADE : TINT
	{
		GRADE_STAR,
		GRADE_A,
		GRADE_B,
		GRADE_C,
		GRADE_D,
		GRADE_F,
		GRADE_NUMOF
	};

	static constexpr TINT NUM_SMOKE_PUFFS = 3;

	//-----------------------------------------------------------------------------
	// One row of the table: a name and a rolling score (vtable 0071b8e0)
	//-----------------------------------------------------------------------------
	class ScoreStat
	    : public Toshi::T2GUIElement
	{
	public:
		enum STATE : TINT
		{
			STATE_HIDDEN,
			STATE_TICKING,
			STATE_DONE,
		};

	public:
		ScoreStat();
		~ScoreStat();

		// Creates the row, returns TFALSE if a_wszName is TNULL
		TBOOL Create( const TWCHAR* a_wszName, TINT a_iScore );

		void Update( TFLOAT a_fDeltaTime );
		void SetState( STATE a_eState );

		STATE  GetState() const { return m_eState; }
		TFLOAT GetScale() const { return m_fScale; }
		void   SetScale( TFLOAT a_fScale ) { m_fScale = a_fScale; }

	private:
		Toshi::T2GUIRectangle m_oBacking;      // 0x54, "ScoreBacking"
		AGUI2ScoreTicker      m_oTicker;       // 0xcc
		AGUI2TextBox          m_oName;         // 0x1c8
		AGUI2TextBox          m_oScore;        // 0x240
		TWCHAR                m_wszScore[ 8 ]; // 0x2b8
		STATE                 m_eState;        // 0x2c8
		TFLOAT                m_fTime;         // 0x2cc
		TFLOAT                m_fScale;        // 0x2d0, scale of the name
	};

public:
	AAnticScoreState( AMiniGameState::S1* a_pResults );
	~AAnticScoreState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;

	STATE GetState() const { return m_eState; }

private:
	STATE                 m_eState;                    // 0xa4
	TFLOAT                m_fTimer;                    // 0xa8
	AGUI2TextBox          m_oTitle;                    // 0xac, name of the minigame
	AGUI2TextBox          m_oSubtitle;                 // 0x124, locale 0xf9
	Toshi::T2GUIRectangle m_oBoard;                    // 0x19c, "Result_Wood"
	Toshi::T2GUIRectangle m_oIcon;                     // 0x214, icon of the minigame
	Toshi::T2GUIRectangle m_oGrade;                    // 0x28c
	Toshi::T2GUIRectangle m_oBrandIron;                // 0x304, "Brand_Iron"
	Toshi::T2GUIRectangle m_aSmoke[ NUM_SMOKE_PUFFS ]; // 0x37c, "Smoke"
	AGUI2TextBox          m_oGradeText;                // 0x4e4
	AGUI2TextBox          m_oGradeTextFlash;           // 0x55c, grows and fades out (grades A and better)
	AGUI2TextBox          m_oResultText;               // 0x5d4, won/draw/lost (never added to the GUI)
	TBOOL                 m_bQuitMiniGame;             // 0x64c
	TBOOL                 m_bRemove;                   // 0x64d
	TBOOL                 m_bContinue;                 // 0x64e, OK was pressed on the "press OK" text
	ScoreStat*            m_pStats;                    // 0x650
	TINT                  m_iNumStats;                 // 0x654
	AMiniGameState::S1*   m_pResults;                  // 0x658
	Toshi::TVector2       m_vecBrandTarget;            // 0x65c
	TINT                  m_iCurrentStat;              // 0x664
	GRADE                 m_eGrade;                    // 0x668
	TWCHAR                m_wszPressOK[ 128 ];         // 0x66c
	TBOOL                 m_bGradeFaded;               // 0x76c
	AGUI2TextBox          m_oPressOK;                  // 0x770
};

TSTATICASSERT( sizeof( AAnticScoreState::ScoreStat ) == 0x2d4 );
TSTATICASSERT( sizeof( AAnticScoreState ) == 0x7e8 );
