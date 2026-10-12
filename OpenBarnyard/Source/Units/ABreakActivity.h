#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Breaks the unit of the category apart ("breaktype"): spawns a "unitname"
// unit for every model of its type at the unit and destroys the unit.
//-----------------------------------------------------------------------------
class ABreakActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ABreakActivity, AInteractionActivity );

	// "breaktype", order of the name table at 0x00775254
	using BREAKTYPE = TINT;
	enum BREAKTYPE_ : BREAKTYPE
	{
		BREAKTYPE_FALLAPPART, // "fallappart"
		BREAKTYPE_NUMOF,
	};

public:
	ABreakActivity();
	~ABreakActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	BREAKTYPE        m_eBreakType;  // 0x08, "breaktype"
	TFLOAT           m_fUnk;        // 0x0c, 5.0f, see Create
	Toshi::TPString8 m_strUnitName; // 0x10, "unitname", type of the pieces
};

TSTATICASSERT( sizeof( ABreakActivity ) == 0x14 );
