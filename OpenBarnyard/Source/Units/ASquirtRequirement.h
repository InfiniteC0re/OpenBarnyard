#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The player has squirted at least "numsquirted" units (APlayerSimProfile).
// TClass 0x0078afbc, vtable 0x0072ac80, size 0x50.
//-----------------------------------------------------------------------------
class ASquirtRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ASquirtRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject, m_iNumSquirted isn't initialised
	ASquirtRequirement() = default;
	~ASquirtRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TINT m_iNumSquirted; // 0x4c, "numsquirted"
};

TSTATICASSERT( sizeof( ASquirtRequirement ) == 0x50 );
