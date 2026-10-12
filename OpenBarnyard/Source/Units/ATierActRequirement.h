#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The story is past the tier "tier" of the act "act".
// TClass 0x0078b07c, vtable 0x0072ad68, size 0x54.
//-----------------------------------------------------------------------------
class ATierActRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ATierActRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ATierActRequirement()
	    : m_iTier( -1 )
	    , m_iAct( -1 )
	{}

	~ATierActRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TINT m_iTier; // 0x4c, "tier"
	TINT m_iAct;  // 0x50, "act"
};

TSTATICASSERT( sizeof( ATierActRequirement ) == 0x54 );
