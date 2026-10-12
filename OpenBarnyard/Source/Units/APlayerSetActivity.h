#pragma once
#include "AInteractionActivity.h"

class APlayerSet;

//-----------------------------------------------------------------------------
// Fills a player set ("playerset", the current one if not set) with the units
// of the category: the player unit as the human player and the other units as
// AI players, plus "additionalaiplayers" AI players.
//-----------------------------------------------------------------------------
class APlayerSetActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( APlayerSetActivity, AInteractionActivity );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE        = 0,
		FLAGS_ADD_PLAYERS = BITFLAG( 1 ), // always set by Create
		FLAGS_RESET       = BITFLAG( 2 ), // always set by Create, resets the player set first
	};

public:
	APlayerSetActivity();
	~APlayerSetActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	// Adds the player unit of the category as the human player of a_pPlayerSet (a_bHumans) or
	// the other units as AI players.
	void AddPlayers( AInteractionSequence* a_pSequence, APlayerSet* a_pPlayerSet, TBOOL a_bHumans );

	// Number of units of the category that aren't the player.
	TINT GetNumAIUnits( AInteractionSequence* a_pSequence );

private:
	TINT  m_iPlayerSet;              // 0x08, "playerset", -1 for the current player set
	TINT  m_iNumAdditionalAIPlayers; // 0x0c, "additionalaiplayers"
	FLAGS m_eFlags;                  // 0x10
};

TSTATICASSERT( sizeof( APlayerSetActivity ) == 0x14 );
