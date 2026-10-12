#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Compares ("compare") the high score of the minigame "gameindex" with
// "highscoreval". The "minigame" variant ("mingamesplayed", "maxgamesplayed",
// "winratio") is read but never evaluated.
// TClass 0x0078ad7c, vtable 0x0072a7f8, size 0x6c.
//-----------------------------------------------------------------------------
class AMiniGameResultRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AMiniGameResultRequirement, AInteractionRequirement );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE      = 0,
		FLAGS_MINIGAME  = BITFLAG( 0 ), // "minigame" is set
		FLAGS_HIGHSCORE = BITFLAG( 1 ), // "minigame" isn't set, "gameindex"
	};

public:
	AMiniGameResultRequirement();
	~AMiniGameResultRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8       m_strMiniGame;     // 0x4c, "minigame"
	FLAGS                  m_uiFlags;         // 0x50
	TINT                   m_iMinGamesPlayed; // 0x54, "mingamesplayed"
	TINT                   m_iMaxGamesPlayed; // 0x58, "maxgamesplayed"
	TFLOAT                 m_fWinRatio;       // 0x5c, "winratio"
	AInteractionComparison m_oCompare;        // 0x60, "compare"
	TINT                   m_iGameIndex;      // 0x64, "gameindex"
	TFLOAT                 m_fHighScore;      // 0x68, "highscoreval"
};

TSTATICASSERT( sizeof( AMiniGameResultRequirement ) == 0x6c );
