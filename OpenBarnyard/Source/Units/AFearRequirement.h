#pragma once
#include "AInteractionRequirement.h"

class AUnitType;

//-----------------------------------------------------------------------------
// The participants are ("fear" true) or aren't scared of the units of type
// "unittype": they know about an AInteractionFearEvent of that type.
// TClass AFearRequirement, vtable 0x0072a434, size 0x58.
//-----------------------------------------------------------------------------
class AFearRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AFearRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AFearRequirement()
	    : m_iUnk4c( 0 )
	    , m_pUnitType( TNULL )
	    , m_bFear( TFALSE )
	{
		m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;
	}

	~AFearRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TINT       m_iUnk4c;    // 0x4c, never used
	AUnitType* m_pUnitType; // 0x50, "unittype"
	TBOOL      m_bFear;     // 0x54, "fear"
};

TSTATICASSERT( sizeof( AFearRequirement ) == 0x58 );
