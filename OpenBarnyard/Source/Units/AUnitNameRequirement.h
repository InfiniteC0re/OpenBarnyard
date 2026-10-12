#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Only the unit registered under the name "match" (AUnitNameManager) can take the role.
//-----------------------------------------------------------------------------
class AUnitNameRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AUnitNameRequirement, AInteractionRequirement );

public:
	AUnitNameRequirement();
	~AUnitNameRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strMatch; // 0x4c, "match"
};

TSTATICASSERT( sizeof( AUnitNameRequirement ) == 0x50 );
