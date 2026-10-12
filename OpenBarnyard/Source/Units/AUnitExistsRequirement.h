#pragma once
#include "AInteractionRequirement.h"

class AUnitType;

//-----------------------------------------------------------------------------
// Satisfied while the number of instances of "unittype" is within
// "minpopulation" and "maxpopulation" (a negative value means no limit).
//-----------------------------------------------------------------------------
class AUnitExistsRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AUnitExistsRequirement, AInteractionRequirement );

public:
	AUnitExistsRequirement();
	~AUnitExistsRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	AUnitType* m_pUnitType;      // 0x4c, "unittype"
	TINT       m_iMinPopulation; // 0x50, "minpopulation"
	TINT       m_iMaxPopulation; // 0x54, "maxpopulation"
};

TSTATICASSERT( sizeof( AUnitExistsRequirement ) == 0x58 );
