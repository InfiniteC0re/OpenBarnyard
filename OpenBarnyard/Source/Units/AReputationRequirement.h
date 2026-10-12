#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit's reputation (AUnitProfileComponent; the reputation the player gave
// it if "playerreputation") has every flag of "flags" ("romeo", "badass", ...).
// TClass 0x0078aefc, vtable 0x0072aa74, size 0x54.
//-----------------------------------------------------------------------------
class AReputationRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AReputationRequirement, AInteractionRequirement );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE              = 0,
		FLAGS_PLAYER_REPUTATION = BITFLAG( 0 ), // "playerreputation"
	};

public:
	// NOTE: inlined into CreateObject
	AReputationRequirement()
	    : m_uiReputation( 0 )
	    , m_uiFlags( FLAGS_NONE )
	{}

	~AReputationRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TUINT m_uiReputation; // 0x4c, "flags", AUnitFlagSetsManager::FLAGSET_REPUTATIONS mask
	FLAGS m_uiFlags;      // 0x50
};

TSTATICASSERT( sizeof( AReputationRequirement ) == 0x54 );
