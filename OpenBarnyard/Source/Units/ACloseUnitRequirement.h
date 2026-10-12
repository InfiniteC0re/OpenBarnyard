#pragma once
#include "AInteractionRequirement.h"

class AUnitType;

//-----------------------------------------------------------------------------
// A unit can take the role if a (visible) unit of "unittype", optionally the one
// named "unitname", is within "maxradius" of it. For the player's type only the
// distance to the player counts.
//-----------------------------------------------------------------------------
class ACloseUnitRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ACloseUnitRequirement, AInteractionRequirement );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE   = 0,
		FLAGS_PLAYER = BITFLAG( 0 ), // "unittype" is the player
	};

public:
	ACloseUnitRequirement();
	~ACloseUnitRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	AUnitType*       m_pUnitType;   // 0x4c, "unittype"
	TFLOAT           m_fMaxRadius;  // 0x50, "maxradius"
	Toshi::TPString8 m_strUnitName; // 0x54, "unitname"
	FLAGS            m_uiFlags;     // 0x58
};

TSTATICASSERT( sizeof( ACloseUnitRequirement ) == 0x5c );
