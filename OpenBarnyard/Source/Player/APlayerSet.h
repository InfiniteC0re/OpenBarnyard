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

class APlayerHumanSlot
{
public:
	APlayerHumanSlot( Toshi::TInputDeviceController* a_pController = TNULL, TBOOL a_bUsed = TFALSE );
	APlayerHumanSlot( const APlayerHumanSlot& a_rcOther );
	~APlayerHumanSlot();

	void Reset();

	APlayerHumanSlot& operator=( const APlayerHumanSlot& a_rcOther );

public:
	TINT                           m_Unk1;
	Toshi::TInputDeviceController* m_pController;
	Toshi::TPString8               m_strUnk;
	TBOOL                          m_bUsed;
};

class APlayerSet
{
public:
	APlayerSet();
	virtual ~APlayerSet();

	void Reset();

	TINT              AddAIPlayer();
	APlayerHumanSlot* AddHumanPlayer( Toshi::TInputDeviceController* a_pController );

	void MakeTeamsFair();

	TBOOL IsPlayerSlotUsed( TINT a_iSlot ) const;

private:
	Toshi::TPString8                                   m_strUnk;
	Toshi::T2Vector<APlayerHumanSlot, MAX_NUM_PLAYERS> m_vecHumanSlots;
	APLAYERTEAM                                        m_aPlayerTeams[ MAX_NUM_PLAYERS ];
	TINT                                               m_iNumAIPlayers;
	TINT                                               m_iNumHumanPlayers;
};
