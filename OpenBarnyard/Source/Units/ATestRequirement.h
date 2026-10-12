#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Always satisfied, Evaluate does nothing.
// TClass 0x0078b04c, vtable 0x0072ad30, size 0x4c.
//-----------------------------------------------------------------------------
class ATestRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ATestRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ATestRequirement() = default;
	~ATestRequirement();

	virtual void Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE {} // +0x1c, DOES_NOTHING
};

TSTATICASSERT( sizeof( ATestRequirement ) == 0x4c );
