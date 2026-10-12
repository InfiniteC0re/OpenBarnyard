#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit's interests (AUnitProfileComponent) have every flag of "flags".
// TClass 0x0078acbc, vtable 0x0072a638, size 0x50.
//-----------------------------------------------------------------------------
class AInterestRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AInterestRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AInterestRequirement()
	    : m_uiInterests( 0 )
	{}

	~AInterestRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TUINT m_uiInterests; // 0x4c, "flags", AUnitFlagSetsManager::FLAGSET_INTERESTS mask
};

TSTATICASSERT( sizeof( AInterestRequirement ) == 0x50 );
