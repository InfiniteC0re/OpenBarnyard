#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The barn is open (see IsBarnOpen).
// TClass 0x0078a9bc, vtable 0x0072a188, size 0x4c.
//-----------------------------------------------------------------------------
class ABarnOpenRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ABarnOpenRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ABarnOpenRequirement() = default;
	~ABarnOpenRequirement();

	virtual void Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

	// Whether the barn is open at night at the absolute quest tier a_iAbsTier (the current one if -1):
	// past the barn detailing quest's tier, except for the tiers between the "Flowers For Ben" and the
	// "Round Up Chicks" quests and the coyote tipping quest's tier
	static TBOOL IsBarnOpenAtNight( TINT a_iAbsTier );

	// Whether the barn is open: always in the barn explore minigame, otherwise during the day or
	// if IsBarnOpenAtNight (never in ACoinPickupMicroGame)
	static TBOOL IsBarnOpen( TINT a_iAbsTier );
};

TSTATICASSERT( sizeof( ABarnOpenRequirement ) == 0x4c );
