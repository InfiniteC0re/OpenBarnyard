#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Adds or removes the quest icon (exclamation/question mark) of the unit
// "unitname" or of the unit of the category ("action").
//-----------------------------------------------------------------------------
class AIconActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AIconActivity, AInteractionActivity );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE              = 0,
		FLAGS_UNITNAME          = BITFLAG( 3 ), // "unitname" is set
		FLAGS_QUESTIONREMOVE    = BITFLAG( 4 ), // "action" "questionremove"
		FLAGS_QUESTIONADD       = BITFLAG( 5 ), // "action" "questionadd"
		FLAGS_EXCLAMATIONREMOVE = BITFLAG( 6 ), // "action" "exclamationremove"
		FLAGS_EXCLAMATIONADD    = BITFLAG( 7 ), // "action" "exclamationadd", Update does nothing with it
	};

public:
	AIconActivity();
	~AIconActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	FLAGS            m_uiFlags;     // 0x08
	Toshi::TPString8 m_strUnitName; // 0x0c, "unitname"
};

TSTATICASSERT( sizeof( AIconActivity ) == 0x10 );
