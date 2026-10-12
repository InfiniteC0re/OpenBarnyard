#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The player's steer is of the class "steer" ("APlayerSteer", "ABikeSteer", ...).
// TClass 0x0078b01c, vtable 0x0072acf4, size 0x50.
//-----------------------------------------------------------------------------
class ASteerRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ASteerRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ASteerRequirement() = default;
	~ASteerRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strSteer; // 0x4c, "steer"
};

TSTATICASSERT( sizeof( ASteerRequirement ) == 0x50 );
