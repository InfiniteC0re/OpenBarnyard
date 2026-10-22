#pragma once
#include "AHighScoreRecord.h"
#include "GUI/AHighScoreChart.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: the high score charts of the games (minigames, races, ...), loaded
// from Data\MinigameData\HighScoreChart\highscore.trb and saved with the game.
// The minigames add the scores of the players, the chart of the last added
// score is shown over the game (AHighScoreChart). Record RECORD_STARS is the
// champion chart: the number of stars of each character (Pip's chart).
// Created by AGameSystemManager.
// TClass 00780acc, vtable 00718ce4, size 0x36ac, singleton DAT_00780afc.
// NOTE: the record functions don't use 'this' in the original (they're
// __stdcall functions of the singleton), they're static here.
//-----------------------------------------------------------------------------
class MODAPI AHighScoreManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AHighScoreManager>
{
public:
	TDECLARE_CLASS( AHighScoreManager, Toshi::TTask );

	using Record = AHighScoreRecord;

	static constexpr TINT NUM_RECORDS = 81;
	static constexpr TINT NUM_SCORES  = Record::NUM_SCORES;
	static constexpr TINT NUM_GRADES  = 6;

	// NOTE: Load/Save copy 0x17e8 bytes from 0x0077ea70, which is 90 records
	static constexpr TINT MAX_RECORDS = 90;

	// The record that counts the stars (grade 0) of the characters, Pip's champion chart
	static constexpr TINT RECORD_STARS = 80;

	// Player ids of the human players in Record::aPlayers, the others are the locale ids of the characters
	static constexpr TINT PLAYER_1 = -2;
	static constexpr TINT PLAYER_2 = -1;

	using GRADE = TINT;
	enum GRADE_ : GRADE
	{
		GRADE_STAR,
		GRADE_A,
		GRADE_B,
		GRADE_C,
		GRADE_D,
		GRADE_F,
		GRADE_NUMOF
	};

public:
	AHighScoreManager();
	~AHighScoreManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Unknown virtual of the class, no override known
	virtual void Unknown1() {} // +0x38, DOES_NOTHING

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Enables showing the chart when a score is added, disabling it hides the chart
	void SetChartEnabled( TBOOL a_bEnabled );

	// Shows the chart of the last record a score was added to and returns the rank
	// of the score in a_rOutRank. a_bClearRank clears the rank. Returns TFALSE if no
	// score was added.
	TBOOL ShowLastRecord( TINT& a_rOutRank, TBOOL a_bClearRank );

	// Clears and hides the chart
	void HideChart();

	// Keeps a_fScore of a_iPlayer as the score of a_iRecord: the best score is
	// added once a worse one is set or after a second without a better one
	void SetPendingScore( TINT a_iRecord, TFLOAT a_fScore, TINT a_iPlayer );

	// Forgets the last added score (a new minigame is started)
	void ClearLastRecord();

	// Updates the champion chart (RECORD_STARS) and shows it
	void ShowChampionChart();

	// Updates the champion chart (RECORD_STARS) and copies it to a_pOutRecord
	static Record* GetChampionChart( Record* a_pOutRecord );

	// Clears the records and loads the default scores and the grades from highscore.trb.
	// The Pip latches are cleared unless a_bInitialising is set.
	static TBOOL ScoreReset( TBOOL a_bInitialising );

	// Adds a_fScore of a_iPlayer to a_iRecord if a_bAdd is set and the score is good
	// enough. With a_bUnique, the player only keeps its best score. Returns TFALSE if
	// the score isn't in the chart.
	static TBOOL AddScore( TINT a_iRecord, TFLOAT a_fScore, TBOOL a_bAdd, TINT a_iPlayer, TBOOL a_bUnique );

	// $Barnyard: FUNCTION 0041c210
	static Record* GetRecord( TINT a_iRecord ) { return &ms_aRecords[ a_iRecord ]; }

	// $Barnyard: FUNCTION 0041c280
	static TFLOAT GetGradeLimit( TINT a_iRecord, GRADE a_eGrade ) { return ms_aGradeLimits[ a_iRecord ][ a_eGrade ]; }

	// Returns the grade of a_fScore in the record a_iRecord, GRADE_F if it's worse than every limit
	static GRADE GetGrade( TINT a_iRecord, TFLOAT a_fScore );

	// Returns the grade of a_fScore in the last record a score was added to, GRADE_F if none
	static GRADE GetLastRecordGrade( TFLOAT a_fScore );

	// Returns how many records (excluding RECORD_STARS) a_iPlayer has a star in
	static TINT GetNumStars( TINT a_iPlayer );

	// Whether a human player has a star in a_iRecord
	static TBOOL HasHumanStar( TINT a_iRecord );

	// Returns the rank of the last score SetPendingScore added and clears it, -1 if none
	static TINT PopLastRank();

	// Returns the previous rank of the player of the last score SetPendingScore added and clears it, -1 if none
	static TINT PopLastPreviousRank();

	// Updates the champion chart (RECORD_STARS) with the stars of the players
	static void UpdateChampionChart();

	// Saves/loads the records (0x17e8 bytes)
	static TBOOL Save( void* a_pBuffer, TINT a_iSize );
	static TBOOL Load( const void* a_pBuffer, TINT a_iSize );

private:
	inline static Record ms_aRecords[ MAX_RECORDS ];                   // 0x0077ea70
	inline static TFLOAT ms_aGradeLimits[ NUM_RECORDS ][ NUM_GRADES ]; // 0x00780258
	inline static TBOOL  ms_bLoaded;                                   // 0x00780ac8
	inline static TBOOL  ms_bShowChart;                                // 0x00780ac9, a score was added, the chart should be shown
	inline static TINT   ms_iShowRecord       = -1;                    // 0x00772790, the record of the chart to show
	inline static TINT   ms_iLastRecord       = -1;                    // 0x00772794, the record a score was last added to
	inline static TINT   ms_iLastRank         = -1;                    // 0x00772798, see PopLastRank
	inline static TINT   ms_iLastPreviousRank = -1;                    // 0x0077279c, see PopLastPreviousRank
	inline static TINT   ms_iPendingRecord    = -1;                    // 0x007727a0, see SetPendingScore
	inline static TINT   ms_iPendingPlayer    = PLAYER_1;              // 0x007727a4

	AHighScoreChart m_oChart;        // 0x0020
	TBOOL           m_bChartEnabled; // 0x369c
	TFLOAT          m_fPendingScore; // 0x36a0
	TFLOAT          m_fPendingTimer; // 0x36a4
	TFLOAT          m_fChartTimer;   // 0x36a8
};
