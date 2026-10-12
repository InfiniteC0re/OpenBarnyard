#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit's relationships (AUnitProfileComponent) have every flag of "flags"
// ("know", "like", "love", "rival", ...), or none if there are no flags.
// TClass 0x0078ae9c, vtable 0x0072a9fc, size 0x50.
//-----------------------------------------------------------------------------
class ARelationshipRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ARelationshipRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ARelationshipRequirement()
	    : m_uiFlags( 0 )
	{}

	~ARelationshipRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TUINT m_uiFlags; // 0x4c, "flags", AUnitFlagSetsManager::FLAGSET_RELATIONSHIPS mask
};

TSTATICASSERT( sizeof( ARelationshipRequirement ) == 0x50 );
