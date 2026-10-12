#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Takes the regrowable object (fruit, crate, milk... see ARegrowthManager) of
// one of the "type"s closest to the units of the category, within "maxradius",
// and "make"s a unit of it, "regrow"s or "destroy"s it.
//-----------------------------------------------------------------------------
class AMakeInstanceActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AMakeInstanceActivity, AInteractionActivity );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE    = 0,
		FLAGS_MAKE    = BITFLAG( 0 ), // "make", spawns a unit of the object
		FLAGS_DESTROY = BITFLAG( 1 ), // "destroy"
		FLAGS_REGROW  = BITFLAG( 2 ), // "regrow"
	};

public:
	AMakeInstanceActivity();
	~AMakeInstanceActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	FLAGS  m_eFlags;        // 0x08, FLAGS_MAKE | FLAGS_REGROW by default
	TUINT  m_auiTypes[ 2 ]; // 0x0c, "type", mask of the ARegrowthManager object types (ARegrowthManager::TypeMask::FromName)
	TFLOAT m_fMaxRadius;    // 0x14, "maxradius"
};

TSTATICASSERT( sizeof( AMakeInstanceActivity ) == 0x18 );
