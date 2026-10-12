#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The gopher shop is open (see IsGopherShopOpen).
// TClass 0x0078ac5c, vtable 0x0072a5c8, size 0x4c.
//-----------------------------------------------------------------------------
class AGopherOpenRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AGopherOpenRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AGopherOpenRequirement() = default;
	~AGopherOpenRequirement();

	virtual void Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

	// Whether the gopher shop is open: it's night (not in ACoinPickupMicroGame) and the
	// barn detailing quest got far enough
	static TBOOL IsGopherShopOpen();
};

TSTATICASSERT( sizeof( AGopherOpenRequirement ) == 0x4c );
