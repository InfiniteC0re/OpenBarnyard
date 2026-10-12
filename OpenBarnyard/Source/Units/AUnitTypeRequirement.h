#pragma once
#include "AInteractionRequirement.h"

class AUnitType;

//-----------------------------------------------------------------------------
// Only units of the type "match" (the primary unit's type if not set) can take
// the role, optionally also of the unit class "class" and the model "model".
//-----------------------------------------------------------------------------
class AUnitTypeRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AUnitTypeRequirement, AInteractionRequirement );

public:
	AUnitTypeRequirement();
	~AUnitTypeRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;             // +0x0c
	virtual void  GetUnitTypeNames( AInteractionUnitTypeNames* a_pNames ) OVERRIDE; // +0x18
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE;   // +0x1c

private:
	AUnitType*           m_pUnitType; // 0x4c, "match"
	TINT                 m_iSkin;     // 0x50, "skin", raw property value, unused
	Toshi::TPString8     m_strModel;  // 0x54, "model", as "data/models/<model>.tmd"
	const Toshi::TClass* m_pClass;    // 0x58, "class"
};

TSTATICASSERT( sizeof( AUnitTypeRequirement ) == 0x5c );
