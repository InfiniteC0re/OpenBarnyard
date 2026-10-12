#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// With "taker" (and "giver"): the units of the taker and giver roles gave each
// other a gift (AInteractionGiftEvent) between "mintimeheld" and "maxtimeheld".
// Otherwise: the unit holds a gift (AGiftComponent) of the interaction "match"
// ("any" for every gift) for between "mintimeheld" and "maxtimeheld" seconds.
// TClass 0x0078ac2c, vtable 0x0072a55c, size 0x68.
//-----------------------------------------------------------------------------
class AGiftRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AGiftRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AGiftRequirement()
	    : m_eTaker( AInteractionSequence::CATEGORY_PRIMARY )
	    , m_eGiver( AInteractionSequence::CATEGORY_SECONDARY )
	    , m_bTakerAndGiver( TFALSE )
	    , m_fMinTimeHeld( 0.0f )
	    , m_fMaxTimeHeld( -1.0f )
	    , m_bAny( TFALSE )
	{}

	~AGiftRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8               m_strMatch;       // 0x4c, "match"
	AInteractionSequence::CATEGORY m_eTaker;         // 0x50, "taker", also the index of the unit
	AInteractionSequence::CATEGORY m_eGiver;         // 0x54, "giver", also the index of the unit
	TBOOL                          m_bTakerAndGiver; // 0x58, "taker" is set
	TFLOAT                         m_fMinTimeHeld;   // 0x5c, "mintimeheld"
	TFLOAT                         m_fMaxTimeHeld;   // 0x60, "maxtimeheld", no limit if <= 0
	TBOOL                          m_bAny;           // 0x64, "match" is "any"
};

TSTATICASSERT( sizeof( AGiftRequirement ) == 0x68 );
