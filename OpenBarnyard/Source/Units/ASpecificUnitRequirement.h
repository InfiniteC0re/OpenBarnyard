#pragma once
#include "AInteractionRequirement.h"

#include <Toshi/TString8.h>

class AUnitType;

//-----------------------------------------------------------------------------
// The unit is of the type "type" ("screenname" is read but unused).
// TClass 0x0078af8c, vtable 0x0072ac38, size 0x5c.
//-----------------------------------------------------------------------------
class ASpecificUnitRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ASpecificUnitRequirement, AInteractionRequirement );

public:
	ASpecificUnitRequirement();
	~ASpecificUnitRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TString8 m_strScreenName; // 0x4c, "screenname"
	AUnitType*      m_pUnitType;     // 0x58, "type"
};

TSTATICASSERT( sizeof( ASpecificUnitRequirement ) == 0x5c );
