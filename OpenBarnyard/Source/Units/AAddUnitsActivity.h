#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Adds the unit "unitname" ("player" for the player) to the sequence, or a
// suitable unit near the primary unit if no name is set.
//-----------------------------------------------------------------------------
class AAddUnitsActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AAddUnitsActivity, AInteractionActivity );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE  = 0,
		FLAGS_BLOCK = BITFLAG( 0 ), // "block", wait until the unit can be added
		FLAGS_FORCE = BITFLAG( 1 ), // "force", take the unit out of its sequence
	};

public:
	AAddUnitsActivity();
	~AAddUnitsActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	FLAGS            m_uiFlags;     // 0x08
	Toshi::TPString8 m_strUnitName; // 0x0c, "unitname"
};

TSTATICASSERT( sizeof( AAddUnitsActivity ) == 0x10 );
