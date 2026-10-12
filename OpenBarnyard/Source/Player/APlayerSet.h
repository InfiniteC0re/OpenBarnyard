#pragma once
#include "GameInterface/ASplitScreen.h"

#include <Toshi/T2Vector.h>
#include <Toshi/TPString8.h>
#include <Input/TInputDeviceController.h>
#include <Toshi/TRefCounted.h>

class AUnitProxy;

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
	Toshi::TRef<AUnitProxy>        m_pUnitProxy; // proxy of the unit of the player, set by APlayerSet::SetPlayerUnit
	Toshi::TInputDeviceController* m_pController;
	Toshi::TPString8               m_strUnitType; // the unit type (or name) the slot plays
	TUINT8                         m_uiFlags;
};

class APlayerSet
{
public:
	APlayerSet();
	virtual ~APlayerSet();

	// Add AI/Human players
	TINT AddAIPlayer();
	// NOTE: a_iUnused isn't read on PC, every retail caller passes -1
	APlayerSlot* AddHumanPlayer( Toshi::TInputDeviceController* a_pController, TINT a_iUnused );

	// Get info about slots
	APlayerSlot* GetPlayerSlot( TINT a_iSlot );
	APlayerSlot* GetAIPlayerSlot( TINT a_iIndex ); // slot of the a_iIndex-th AI player or TNULL
	TINT         GetPlayerSlotIndexByDevice( Toshi::TInputDevice* a_pDevice );
	class AUnit* GetPlayerUnit( TINT a_iSlot );
	class AUnit* GetHumanPlayerUnit( TINT a_iIndex ); // unit of the a_iIndex-th slot used by a human

	// Makes a_pUnit the unit of the slot a_iSlot, or of the first slot without a unit if a_iSlot is negative
	TBOOL SetPlayerUnit( class AUnit* a_pUnit, TINT a_iSlot );

	// Clears the units of all the slots
	void  ResetPlayerUnits();
	TBOOL IsPlayerSlotUsedByHuman( TINT a_iSlot ) const;
	TBOOL IsPlayerSlotUsedByAI( TINT a_iSlot ) const;

	TINT GetTotalPlayerCount() const { return m_iNumAIPlayers + m_iNumHumanPlayers; }
	TINT GetNumAIPlayers() const { return m_iNumAIPlayers; }
	TINT GetHumanPlayerCount() const { return m_iNumHumanPlayers; }

	// $Barnyard: FUNCTION 006203f0
	APLAYERTEAM GetPlayerTeam( TINT a_iSlot ) const { return m_aPlayerTeams[ a_iSlot ]; }

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
