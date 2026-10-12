#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Sets up the item count HUD element (AItemCountHudElement): its texture,
// what it counts ("mode"), the count, the required count and its visibility.
//-----------------------------------------------------------------------------
class AItemCountHudActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AItemCountHudActivity, AInteractionActivity );

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                = 0,
		FLAGS_TEXTURE             = BITFLAG( 1 ),  // "texture"
		FLAGS_SETCOUNT            = BITFLAG( 2 ),  // "setcount"
		FLAGS_ADDCOUNT            = BITFLAG( 3 ),  // "addcount", NOTE: set whenever "setcount" isn't
		FLAGS_SHOW                = BITFLAG( 4 ),  // "setvisible" TTRUE
		FLAGS_HIDE                = BITFLAG( 5 ),  // "setvisible" TFALSE
		FLAGS_SETREQUIRED         = BITFLAG( 6 ),  // "setrequired"
		FLAGS_MODE_MANUAL         = BITFLAG( 7 ),  // "mode" "manual"
		FLAGS_MODE_INVENTORY      = BITFLAG( 8 ),  // "mode" "inventory"
		FLAGS_MODE_NUMJUNK        = BITFLAG( 9 ),  // "mode" "numjunk"
		FLAGS_MODE_NUMTLPARTS     = BITFLAG( 10 ), // "mode" "numtlparts"
		FLAGS_MODE_NUMROCKS       = BITFLAG( 11 ), // "mode" "numrocks"
		FLAGS_MODE_NUMLEAKS       = BITFLAG( 12 ), // "mode" "numleaks"
		FLAGS_REQUIRED            = BITFLAG( 13 ), // "required"
		FLAGS_MODE_SQUIRT         = BITFLAG( 14 ), // "mode" "squirt"
		FLAGS_MODE_NUMFREDDYVEG   = BITFLAG( 15 ), // "mode" "numfreddyveg"
		FLAGS_MODE_NUMFREDDYFRUIT = BITFLAG( 16 ), // "mode" "numfreddyfruit"
	};

public:
	AItemCountHudActivity();
	~AItemCountHudActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	FLAGS            m_uiFlags;     // 0x08
	TINT             m_iCount;      // 0x0c, "setcount" / "addcount"
	TINT             m_iRequired;   // 0x10, "setrequired"
	Toshi::TPString8 m_strTexture;  // 0x14, "texture"
	Toshi::TPString8 m_strRequired; // 0x18, "required"
};

TSTATICASSERT( sizeof( AItemCountHudActivity ) == 0x1c );
