#pragma once
#include "GameInterface/ASplitScreen.h"

#include <Toshi/T2Vector.h>
#include <Toshi/TPString8.h>
#include <Input/TInputDeviceController.h>

#define MAX_NUM_PLAYERS SPLITSCREEN_MAX_NUM_PLAYERS

using APLAYERTEAM = TUINT32;
enum APLAYERTEAM_ : APLAYERTEAM
{
	APLAYERTEAM_TEAM1,
	APLAYERTEAM_TEAM2,
	APLAYERTEAM_NONE
};

class APlayerSlot
{
public:
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE  = 0,
		FLAGS_HUMAN = BITFLAG( 0 ),
		FLAGS_AI    = BITFLAG( 1 ),
	};

public:
	APlayerSlot( Toshi::TInputDeviceController* a_pController = TNULL, FLAGS a_uiFlags = FLAGS_NONE );
	APlayerSlot( const APlayerSlot& a_rcOther );
	~APlayerSlot();

	void Reset();

	APlayerSlot& operator=( const APlayerSlot& a_rcOther );

public:
	TINT                           m_Unk1;
	Toshi::TInputDeviceController* m_pController;
	Toshi::TPString8               m_strUnk;
	TUINT8                         m_uiFlags;
};

class APlayerSet
{
public:
	APlayerSet();
	virtual ~APlayerSet();

	// Add AI/Human players
	TINT         AddAIPlayer();
	APlayerSlot* AddHumanPlayer( Toshi::TInputDeviceController* a_pController );

	// Get info about slots
	APlayerSlot* GetPlayerSlot( TINT a_iSlot );
	TBOOL        IsPlayerSlotUsedByHuman( TINT a_iSlot ) const;
	TBOOL        IsPlayerSlotUsedByAI( TINT a_iSlot ) const;

	TINT GetTotalNumPlayers() const { return m_iNumAIPlayers + m_iNumHumanPlayers; }
	TINT GetNumAIPlayers() const { return m_iNumAIPlayers; }
	TINT GetNumHumanPlayers() const { return m_iNumHumanPlayers; }

	void MakeTeamsFair();
	void FinishUpAddingAIs();
	void Reset();

private:
	Toshi::TPString8                              m_strUnk;
	Toshi::T2Vector<APlayerSlot, MAX_NUM_PLAYERS> m_vecPlayerSlots;
	APLAYERTEAM                                   m_aPlayerTeams[ MAX_NUM_PLAYERS ];
	TINT                                          m_iNumAIPlayers;
	TINT                                          m_iNumHumanPlayers;
};
