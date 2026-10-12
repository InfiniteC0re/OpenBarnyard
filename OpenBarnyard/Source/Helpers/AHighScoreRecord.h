#pragma once

//-----------------------------------------------------------------------------
// A high score chart of AHighScoreManager (0x44 bytes, the table is at 0x0077ea70).
// NOTE: declared outside of AHighScoreManager so AHighScoreChart can use it.
//-----------------------------------------------------------------------------
struct AHighScoreRecord
{
	static constexpr TINT NUM_SCORES = 5;

	// How the score is shown, and whether the lower (_LOW) or the higher (_HIGH) one is better
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_INTEGER_LOW  = 0,
		TYPE_INTEGER_HIGH = 1,
		TYPE_DECIMAL_LOW  = 2,
		TYPE_DECIMAL_HIGH = 3,
		TYPE_TIME_LOW     = 4,
		TYPE_TIME_HIGH    = 5,
	};

	TINT   iIndex;                 // 0x00
	TYPE   eType;                  // 0x04
	TINT   iLastRank;              // 0x08, rank of the last added score, -1 if none
	TINT   iNameId;                // 0x0c, locale id
	TINT   iShortNameId;           // 0x10, locale id, shown under the name if set
	TINT   iHidden;                // 0x14, "GameFlags", not shown in the scorebook unless 0
	TBOOL  bPlayed;                // 0x18, the player has added a score
	TINT   aPlayers[ NUM_SCORES ]; // 0x1c, locale id of the character's name, or AHighScoreManager::PLAYER_1/PLAYER_2
	TFLOAT aScores[ NUM_SCORES ];  // 0x30, 0 if none
};

TSTATICASSERT( sizeof( AHighScoreRecord ) == 0x44 );
