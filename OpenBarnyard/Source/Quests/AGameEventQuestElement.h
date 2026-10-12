#pragma once
#include "AQuestElement.h"
#include "MiniGame/AMiniGameState.h"

#include <Toshi/TString16.h>

//-----------------------------------------------------------------------------
// Quest element completed by playing a minigame ("gameevent"): the minigame,
// its variant and the units playing it, the player has to reach "endstate".
//-----------------------------------------------------------------------------
class AGameEventQuestElement : public AQuestElement
{
public:
	TDECLARE_CLASS( AGameEventQuestElement, AQuestElement );

	static constexpr TINT MAX_NUM_PLAYERS = 4;

	// "endstate"
	using ENDSTATE = TINT;
	enum ENDSTATE_ : ENDSTATE
	{
		ENDSTATE_WIN  = 0,
		ENDSTATE_LOSE = 1,
		ENDSTATE_DRAW = 2,
	};

public:
	AGameEventQuestElement();
	~AGameEventQuestElement();

	//-----------------------------------------------------------------------------
	// AQuestElement
	//-----------------------------------------------------------------------------
	virtual TBOOL    Create( const PBProperties* a_pProperties, TINT* a_pDataOffset ) OVERRIDE;               // +0x0c
	virtual TBOOL    InitData( AInteractionActivityData* a_pData ) OVERRIDE;                                  // +0x10
	virtual TBOOL    GetDescription( const TWCHAR** a_ppOutText ) OVERRIDE;                                   // +0x18
	virtual void     Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ) OVERRIDE; // +0x1c
	virtual TBOOL    Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData ) OVERRIDE; // +0x20
	virtual PROGRESS CheckProgress( AInteractionActivityData* a_pData ) OVERRIDE;                             // +0x28

	// Completes or fails the element once its minigame is over
	void OnMiniGameResult( AInteractionActivityData* a_pData, AMiniGameState::S1* a_pResults );

	// Sets up the players of the minigame a_rcName if it's the minigame of this element
	void OnMiniGameStart( const Toshi::TPString8& a_rcName, AInteractionActivityData* a_pData );

	const Toshi::TPString8& GetMiniGame() const { return m_strMiniGame; }

private:
	// Reads "players"
	TBOOL ReadPlayers( TINT* a_pDataOffset, const PBPropertyValueArray* a_pPlayers );

	// Index of "win", "lose" or "draw", -1 if it's none of them
	static ENDSTATE GetEndState( const Toshi::TPString8& a_rcName );

	PROGRESS* GetResult( AInteractionActivityData* a_pData ) const { return TREINTERPRETCAST( PROGRESS*, TSTATICCAST( TUINT8, a_pData->pData ) + m_iResultOffset ); }

private:
	Toshi::TPString8 m_strMiniGame;                    // 0x5c, "minigame"
	TINT             m_iNumPlayers;                    // 0x60
	Toshi::TPString8 m_astrPlayers[ MAX_NUM_PLAYERS ]; // 0x64, "players"
	TINT             m_iWinPlayer;                     // 0x74, index of "winplayer" in m_astrPlayers or -1
	ENDSTATE         m_eEndState;                      // 0x78, "endstate"
	TINT             m_iVariant;                       // 0x7c, "variant", 1-based
	Toshi::TString16 m_strDescription;                 // 0x80
	TINT             m_iResultOffset;                  // 0x8c, offset of the PROGRESS in the instance data
};

TSTATICASSERT( sizeof( AGameEventQuestElement ) == 0x90 );
