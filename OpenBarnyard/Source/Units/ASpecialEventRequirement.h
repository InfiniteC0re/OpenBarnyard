#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// One of the hardcoded game conditions "type" ("player_in_barn",
// "radio_groundfloor_on", "can_enter_kitchen", "player_is_hero", ...) is met.
// TClass 0x0078af5c, vtable 0x0072ab44, size 0x50.
//-----------------------------------------------------------------------------
class ASpecialEventRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ASpecialEventRequirement, AInteractionRequirement );

	using EVENT = TINT;
	enum EVENT_ : EVENT
	{
		EVENT_NONE                   = 0,
		EVENT_RADIO_GROUNDFLOOR_ON   = 2, // "radio_groundfloor_on"
		EVENT_RADIO_BASEMENT_ON      = 3, // "radio_basement_on"
		EVENT_PLAYER_IN_BARN         = 4, // "player_in_barn"
		EVENT_GOT_APPLE_PIE_ROCK     = 5, // "got_apple_pie_rock"
		EVENT_CAN_ENTER_KITCHEN      = 6, // "can_enter_kitchen"
		EVENT_SUPER_SQUIRT_AVAILABLE = 7, // "super_squirt_available"
		EVENT_PLAYER_IS_CHAMPION     = 8, // "player_is_champion"
		EVENT_PLAYER_IS_HERO         = 9, // "player_is_hero"
	};

public:
	// NOTE: inlined into CreateObject
	ASpecialEventRequirement()
	    : m_eEvent( EVENT_NONE )
	{}

	~ASpecialEventRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

	// Whether the player can go into the kitchen ("Pig Apple Pie" got far enough, it's not night)
	static TBOOL CanEnterKitchen();

	// Whether the super squirt can be got (the quest "Otis Super Squirt" is assigned or Act 2 Tier 3 is done)
	static TBOOL IsSuperSquirtAvailable();

private:
	EVENT m_eEvent; // 0x4c, "type"
};

TSTATICASSERT( sizeof( ASpecialEventRequirement ) == 0x50 );
