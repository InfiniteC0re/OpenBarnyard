#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// A unit can take the role if a unit of "unittype" is attached to its bone
// "parentbonename".
//-----------------------------------------------------------------------------
class AAttachedUnitRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AAttachedUnitRequirement, AInteractionRequirement );

public:
	AAttachedUnitRequirement();
	~AAttachedUnitRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strUnitType;   // 0x4c, "unittype", "none" by default
	Toshi::TPString8 m_strParentBone; // 0x50, "parentbonename", "c_l_clavicle" by default
};

TSTATICASSERT( sizeof( AAttachedUnitRequirement ) == 0x54 );
