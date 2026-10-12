#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// Compares ("compare") the number of bucks ATreasureManager has created
// ("totalcreated") or that are in the world ("currentamount"), or checks that
// ARegrowthManager has regrown the day objects ("egg").
// TClass 0x0078b0dc, vtable 0x0072ade8, size 0x58.
//-----------------------------------------------------------------------------
class ATreasureRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ATreasureRequirement, AInteractionRequirement );

	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_TOTAL_CREATED  = 0, // "totalcreated"
		MODE_CURRENT_AMOUNT = 1, // "currentamount"
		MODE_EGG            = 2, // "egg"
	};

public:
	ATreasureRequirement();
	~ATreasureRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TINT                   m_iAmount;  // 0x4c, "totalcreated" or "currentamount"
	AInteractionComparison m_oCompare; // 0x50, "compare"
	MODE                   m_eMode;    // 0x54, not initialised if no property is set
};

TSTATICASSERT( sizeof( ATreasureRequirement ) == 0x58 );
