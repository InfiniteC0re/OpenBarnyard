#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Adds ("action" "add") or removes ("remove") the relationship "flags" (see
// AUnitFlagSetsManager::FLAGSET_RELATIONSHIPS) to / from the AUnitProfileComponent
// of the units of the category. The other flags of the groups are cleared.
//-----------------------------------------------------------------------------
class ARelationshipActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ARelationshipActivity, AInteractionActivity );

	using ACTION = TUINT;
	enum ACTION_ : ACTION
	{
		ACTION_NONE   = 0,
		ACTION_ADD    = BITFLAG( 0 ), // "add", also adds the "know" flag
		ACTION_REMOVE = BITFLAG( 1 ), // "remove"
	};

public:
	ARelationshipActivity();
	~ARelationshipActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TUINT  m_uiFlags;      // 0x08, "flags"
	TUINT  m_uiGroupFlags; // 0x0c, every flag of the groups of m_uiFlags
	ACTION m_eAction;      // 0x10, "action"
};

TSTATICASSERT( sizeof( ARelationshipActivity ) == 0x14 );
