#pragma once
#include "Units/AInteractionManager.h"

#include <Toshi/TSingleton.h>
#include <Toshi/TGlobalEmitter.h>
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Keyboard cheat codes: hold Tab and type one of the words of ms_aCheats.
// AGameStateController::ProcessInput gives every input event to it first.
// It also keeps the indices of a few interaction requirements that tell how
// far the story is, used by AQuestManager and AMiscFunctions::PlayerCanGetOnBike.
//-----------------------------------------------------------------------------
class ACheatActivator
    : public Toshi::TObject
    , public Toshi::TSingleton<ACheatActivator>
{
public:
	TDECLARE_CLASS( ACheatActivator, Toshi::TObject );

	static constexpr TINT MAX_CODE_LENGTH = 32;

	// Game states a cheat can be entered in, see GetCurrentStateMask
	using STATEMASK = TUINT8;
	enum STATEMASK_ : STATEMASK
	{
		STATEMASK_NONE      = 0,
		STATEMASK_ANY       = BITFLAG( 0 ), // set for every state
		STATEMASK_SIM       = BITFLAG( 1 ), // ASimState, ABarnExploreMinigameState
		STATEMASK_MINIGAME  = BITFLAG( 2 ), // AMiniGameState (no cheat uses it)
		STATEMASK_BONUS     = BITFLAG( 3 ), // AFrontEndBonusState, AFrontEndArtGalleryState
		STATEMASK_ANTICS    = BITFLAG( 4 ), // AFrontEndMiniGameState2
		STATEMASK_MAIN_MENU = BITFLAG( 5 ), // AFrontEndMainMenuState2
		STATEMASK_OTHER     = BITFLAG( 6 ), // any other state
	};

	enum CHEAT
	{
		CHEAT_UNLOCK_MUD_JUMP,        // TIMETOGOTOWORK
		CHEAT_UNLOCK_BARN_POOL,       // DOGSPLAYINGPOOL
		CHEAT_UNLOCK_BIKE_RACE,       // COWSONBIKES
		CHEAT_UNLOCK_CHASING_CHICKS,  // HANGINGWITHOTIS
		CHEAT_UNLOCK_VEGGIE_PATCH,    // ILOVEPUMPKINS
		CHEAT_UNLOCK_COW_TIPPING,     // THATSCALLEDCOWTIPPING
		CHEAT_UNLOCK_BARNTENDER,      // OHMILKME
		CHEAT_MONEY,                  // GIMMESOMELOVE
		CHEAT_ITEMS,                  // IWANTSOME
		CHEAT_UNLOCK_ALL_BONUS_ITEMS, // SHOWMESTUFF
		CHEAT_UNLOCK_ALL_ANTICS,      // FUNANDGAMES
		CHEAT_NUMOF,
	};

	// Indices of interaction requirements looked up once the interactions are loaded
	enum REQUIREMENT
	{
		REQUIREMENT_COOKING_CHEAT_AVAILABLE, // "Cooking Cheat Available"
		REQUIREMENT_DUKE_BARN_POOL_GIVEN,    // "Duke (Night) Barn Pool Given"
		REQUIREMENT_ACT_3_COMPLETE,          // "Act 3 - Complete"
		REQUIREMENT_ACT_1_TIER_0_COMPLETE,   // "Act 1 Tier 0 - Complete"
		REQUIREMENT_PLAYER_HAS_GATE_KEY,     // "Player has Gate Key"
		REQUIREMENT_SECONDARY_HAS_BIKE_KEY,  // "secondary_has_bike_key"
		REQUIREMENT_ROCK_CLOSE_TO_SECONDARY, // "instance_rock_close_to_secondary"
		REQUIREMENT_NUMOF,
	};

	struct CheatCodeRef
	{
		TINT      aKeys[ MAX_CODE_LENGTH ]; // 0x00
		TINT      iLength;                  // 0x80
		TINT      iMessage;                 // 0x84, locale id of the text of the popup
		STATEMASK eAllowedStates;           // 0x88
		TBOOL     bShowPopup;               // 0x89
	};

	using InteractionListener = Toshi::TGlobalListener<ACheatActivator, AInteractionManagerEvent>;

public:
	ACheatActivator();
	~ACheatActivator();

	// Records the key of a_pEvent when it's one of ms_aCheatKeys, returns TFALSE if it's not a gone down event
	TBOOL ProcessCheatCodeInput( const Toshi::TInputInterface::InputEvent* a_pEvent );

	// TRUE while the key that has to be held to enter cheats (Tab) is down
	TBOOL IsCheatKeyDown();

	// Forgets the recorded keys
	TBOOL ResetCheatCodeInput();

	// Forgets the recorded keys and goes back to Act 1 Tier 1
	TBOOL Reset();

	// Copies the act and the tier from AQuestManager
	void UpdateQuestTier();

	// Forces the "Act X Tier Y - Complete" requirements of the tiers before the current one
	void UpdateTierRequirements();

	// Index of the requirement "Act a_iAct Tier a_iTier - Complete", or "Act a_iAct - Complete" if a_iTier is -1
	TINT GetTierRequirement( TINT a_iAct, TINT a_iTier );

	TINT GetRequirement( REQUIREMENT a_eRequirement ) const { return m_aiRequirements[ a_eRequirement ]; }

	// Same as AInteractionManager::ForceRequirementSatisfied without the check of the index
	static TBOOL SetRequirementSatisfied( TINT a_iRequirement, TBOOL a_bSatisfied );

	// Stops forcing the requirement a_iRequirement
	static TBOOL ResetRequirement( TINT a_iRequirement );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devcheat <WORD>" enters the cheat once the sim has been running for a few seconds
	static void Dev_Update();
#endif

private:
	// Records a_iKey and activates the cheat whose code matches the last keys, returns TTRUE if there was one
	TBOOL AddCheatCodeInput( TINT a_iKey );

	// TRUE if the last recorded keys are a_pKeys
	TBOOL IsMatchCheatCode( const TINT* a_pKeys, TINT a_iLength );

	// Activates the cheat a_eCheat if the current state allows it and shows the popup
	void TriggerCheat( CHEAT a_eCheat );

	// Applies the effect of the cheat a_eCheat
	void ActivateCheat( CHEAT a_eCheat );

	// STATEMASK of the current game state
	static STATEMASK GetCurrentStateMask();

	// Looks up the "Act X Tier Y - Complete" and "Act X - Complete" requirements
	void FindTierRequirements();

	void        OnInteractionManagerEvent( const AInteractionManagerEvent& a_rcEvent );
	static void OnInteractionManagerEventCallback( void* a_pReceiver, const AInteractionManagerEvent& a_rcEvent );

public:
	static const TINT         ms_aCheatKeys[ 28 ];
	static const CheatCodeRef ms_aCheats[ CHEAT_NUMOF ];

private:
	TINT                m_iUnk04;                              // 0x04
	TINT                m_iUnk08;                              // 0x08
	TBOOL               m_bUnk0C;                              // 0x0c
	TBOOL               m_bUnk0D;                              // 0x0d
	TBOOL               m_bUnk0E;                              // 0x0e
	TINT*               m_pKeys;                               // 0x10, ring buffer of the recorded keys
	TINT                m_iMaxKeys;                            // 0x14
	TINT                m_iFirstKey;                           // 0x18
	TINT                m_iNumKeys;                            // 0x1c
	TINT                m_iLastCheat;                          // 0x20
	TINT                m_iAct;                                // 0x24
	TINT                m_iTier;                               // 0x28
	InteractionListener m_oInteractionListener;                // 0x2c
	TINT                m_aiRequirements[ REQUIREMENT_NUMOF ]; // 0x3c
	TINT*               m_paiTierRequirements;                 // 0x58, every tier of every act, then the 3 acts
	TINT                m_iNumTierRequirements;                // 0x5c
	TBOOL               m_bUnk60;                              // 0x60
};
