#pragma once
#include "AGUI2TextBox.h"
#include "AGUI2MenuGrid.h"
#include "AGUI2MenuGridTextItem.h"
#include "AGUI2MenuGridRectItem.h"
#include "AGUI2MenuGridTickerItem.h"
#include "Helpers/AHighScoreRecord.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: the high score chart AHighScoreManager shows over the game: the
// name and the icon of the game on a "Result_Wood" board and a row per score
// (rank, face, name, spinning score and grade). The rank of the last added
// score blinks.
// NOTE: the original name is unknown, it's embedded at 0x20 of AHighScoreManager
// (constructor 0063ae40, size 0x367c).
//-----------------------------------------------------------------------------
class AHighScoreChart
{
public:
	AHighScoreChart();
	~AHighScoreChart();

	// Creates the grid of the rows (hidden)
	TBOOL Create();

	// Fills the chart with a_pRecord. a_bUnused isn't used.
	void Show( AHighScoreRecord* a_pRecord, TBOOL a_bUnused );

	// Shows/hides the board and the rows
	void SetVisible( TBOOL a_bVisible );

	// $Barnyard: FUNCTION 0063b160
	TBOOL IsVisible() const { return m_oGrid.IsVisible(); }

	// Clears the texts and the tickers of the rows
	void Clear();

	// Blinks the rank of the last added score
	void Update( TFLOAT a_fDeltaTime );

	// Returns the picture of a_iPlayer: the hide of the main player for the human
	// players, else the face of the character with the locale string a_iPlayer
	static Toshi::T2GUITextureSection* GetPlayerTexture( TINT a_iPlayer );

private:
	void Unlink();

public:
	inline static TFLOAT ms_fBlinkTime = 1.0f; // 0x00779838

private:
	TWCHAR                  m_awszRanks[ AHighScoreRecord::NUM_SCORES ][ 7 ]; // 0x0000
	TCHAR                   PADDING1[ 0xfe ];                                 // 0x0046
	TINT                    m_iBlinkRank;                                     // 0x0144
	TCHAR                   PADDING2[ 0x24 ];                                 // 0x0148
	TBOOL                   m_bUnk16C;                                        // 0x016c
	AGUI2TextBox            m_oTitle;                                         // 0x0170
	AGUI2TextBox            m_oTitleSuffix;                                   // 0x01e8
	Toshi::T2GUIRectangle   m_oBoard;                                         // 0x0260
	Toshi::T2GUIRectangle   m_oIcon;                                          // 0x02d8
	AGUI2TextBox            m_oShortName;                                     // 0x0350
	AGUI2MenuGrid           m_oGrid;                                          // 0x03c8
	AGUI2MenuGridTextItem   m_aUnusedItems[ AHighScoreRecord::NUM_SCORES ];   // 0x048c
	AGUI2MenuGridTextItem   m_aRankItems[ AHighScoreRecord::NUM_SCORES ];     // 0x0ef0
	AGUI2MenuGridRectItem   m_aFaceItems[ AHighScoreRecord::NUM_SCORES ];     // 0x1954
	AGUI2MenuGridTextItem   m_aNameItems[ AHighScoreRecord::NUM_SCORES ];     // 0x1df0
	AGUI2MenuGridTickerItem m_aScoreItems[ AHighScoreRecord::NUM_SCORES ];    // 0x2854
	AGUI2MenuGridRectItem   m_aGradeItems[ AHighScoreRecord::NUM_SCORES ];    // 0x2f84
	Toshi::T2GUIRectangle   m_aBackings[ AHighScoreRecord::NUM_SCORES ];      // 0x3420
	TFLOAT                  m_fBlinkTimer;                                    // 0x3678
};
