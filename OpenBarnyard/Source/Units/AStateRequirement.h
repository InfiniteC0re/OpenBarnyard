#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The current game state is of the class "state".
// TClass 0x0078afec, vtable 0x0072acb8, size 0x50.
//-----------------------------------------------------------------------------
class AStateRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AStateRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AStateRequirement() = default;
	~AStateRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strState; // 0x4c, "state"
};

TSTATICASSERT( sizeof( AStateRequirement ) == 0x50 );
