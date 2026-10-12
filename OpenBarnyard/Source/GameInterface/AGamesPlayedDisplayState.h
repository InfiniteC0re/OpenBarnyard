#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2ScoreTickerAdvanced.h"

#include <GUI/T2GUIRectangle.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Purpose: The scorebook ("scorebook" inventory item). The first page tells
// how many games there are, how many were played and how many stars the
// players got, the next pages list the games (5 per page) with the score of
// the player and its grade (AHighScoreManager).
// TClass 00781e00, vtable 0071bc20.
//-----------------------------------------------------------------------------
class AGamesPlayedDisplayState
    : public AGameState
{
public:
	TDECLARE_CLASS( AGamesPlayedDisplayState, AGameState );

	using INPUT = TUINT32;
	enum INPUT_ : INPUT
	{
		INPUT_NONE     = 0,
		INPUT_UP       = BITFLAG( 0 ),
		INPUT_DOWN     = BITFLAG( 1 ),
		INPUT_PREVIOUS = BITFLAG( 2 ),
		INPUT_NEXT     = BITFLAG( 3 ),
		INPUT_OK       = BITFLAG( 4 ),
		INPUT_CANCEL   = BITFLAG( 5 ),
	};

	static constexpr TINT NUM_ROWS = 5;

public:
	AGamesPlayedDisplayState();
	~AGamesPlayedDisplayState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	void CreateGUI();
	void UpdatePage();

	// Shows the record a_iRecord in the row a_iRow, hides the row if a_iRecord is negative
	void SetRow( TINT a_iRow, TINT a_iRecord );

	// Is the record listed in the scorebook?
	TBOOL IsRecordShown( TINT a_iRecord ) const;

	// Returns the a_iCount'th shown record after/before a_iRecord, -1 if there's none
	TINT FindNextRecord( TINT a_iRecord, TINT a_iCount ) const;
	TINT FindPreviousRecord( TINT a_iRecord, TINT a_iCount ) const;

	// Returns the page (1-based) of m_iRecord, 0 for the first (summary) page
	TINT GetCurrentPage() const;

	// Returns the first record of the page a_iPage (1-based), -1 if there's no such page
	TINT GetPageRecord( TINT a_iPage ) const;

	TINT GetNumPages() const { return Toshi::TMath::CeilToInt( TFLOAT( m_iNumShownRecords ) / TFLOAT( m_iRecordsPerPage ) ); }

private:
	Toshi::T2GUIRectangle    m_oBackground;              // 0x0a4
	Toshi::T2GUIRectangle    m_oScoreCard;               // 0x11c, "Score_Card"
	Toshi::T2GUIRectangle    m_oNavigation;              // 0x194, the arrows and the page number
	Toshi::T2GUIRectangle    m_oPageNumberArea;          // 0x20c
	Toshi::T2GUIRectangle    m_oSummary;                 // 0x284, the first page
	AGUI2TextBox             m_oNumGamesLabel;           // 0x2fc
	AGUI2TextBox             m_oNumPlayedLabel;          // 0x374
	AGUI2TextBox             m_oNumStarsLabel;           // 0x3ec
	Toshi::T2GUIRectangle    m_oNumGamesRow;             // 0x464
	Toshi::T2GUIRectangle    m_oNumPlayedRow;            // 0x4dc
	Toshi::T2GUIRectangle    m_oNumStarsRow;             // 0x554
	AGUI2TextBox             m_oNumGames;                // 0x5cc
	AGUI2TextBox             m_oNumPlayed;               // 0x644
	AGUI2TextBox             m_oNumStars;                // 0x6bc
	AGUI2TextBox             m_oTitle;                   // 0x734
	AGUI2TextBox             m_oPrompt;                  // 0x7ac
	AGUI2TextBox             m_oPageLabel;               // 0x824
	AGUI2TextBox             m_oPageNumber;              // 0x89c
	Toshi::T2GUIRectangle    m_oLeftArrow;               // 0x914
	Toshi::T2GUIRectangle    m_oRightArrow;              // 0x98c
	Toshi::T2GUIRectangle    m_oTable;                   // 0xa04, the pages listing the games
	Toshi::T2GUIRectangle    m_aRows[ NUM_ROWS ];        // 0xa7c
	AGUI2TextBox             m_aGameNames[ NUM_ROWS ];   // 0xcd4
	AGUI2TextBox             m_aUnused[ NUM_ROWS ];      // 0xf2c
	AGUI2ScoreTickerAdvanced m_aScores[ NUM_ROWS ];      // 0x1184
	Toshi::T2GUIRectangle    m_aGrades[ NUM_ROWS ];      // 0x2750
	TWCHAR                   m_wszPrompt[ 256 ];         // 0x29a8
	TWCHAR                   m_wszPageNumber[ 16 ];      // 0x2ba8
	TWCHAR                   m_wszNumGames[ 8 ];         // 0x2bc8
	TWCHAR                   m_wszNumPlayed[ 8 ];        // 0x2bd8
	TWCHAR                   m_wszNumStars[ 8 ];         // 0x2be8
	INPUT                    m_eInput;                   // 0x2bf8
	TINT                     m_iUnk2BFC;                 // 0x2bfc
	TINT                     m_iUnk2C00;                 // 0x2c00
	TFLOAT                   m_fTime;                    // 0x2c04, seconds since the last analog stick page change
	TINT                     m_iUnk2C08;                 // 0x2c08
	TINT                     m_iRecord;                  // 0x2c0c, first record of the page, -1 shows the summary page
	TINT                     m_iShownRecord;             // 0x2c10, m_iRecord when the page was last updated
	TINT                     m_iRecordsPerPage;          // 0x2c14
	TINT                     m_iNumShownRecords;         // 0x2c18
	TINT                     m_iNumRecords;              // 0x2c1c
	TINT                     m_iNumPlayedRecords;        // 0x2c20
	TINT                     m_aPageRecords[ NUM_ROWS ]; // 0x2c24
	TFLOAT                   m_fFinishScoresTime;        // 0x2c38, the scores stop spinning when it runs out
	TFLOAT                   m_fTableWidth;              // 0x2c3c
	TBOOL                    m_bOnlyPlayed;              // 0x2c40, only lists the games that were played
};

TSTATICASSERT( sizeof( AGamesPlayedDisplayState ) == 0x2c44 );
