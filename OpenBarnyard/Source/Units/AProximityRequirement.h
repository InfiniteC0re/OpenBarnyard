#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The first two units are between "mindistance" and "maxdistance" apart.
// Both properties are required, Create fails without them.
// TClass 0x0078ae3c, vtable 0x0072a94c, size 0x54.
//-----------------------------------------------------------------------------
class AProximityRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AProximityRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AProximityRequirement()
	    : m_fMinDistance( 10.0f )
	    , m_fMaxDistance( 10.0f )
	{}

	~AProximityRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TFLOAT m_fMinDistance; // 0x4c, "mindistance"
	TFLOAT m_fMaxDistance; // 0x50, "maxdistance"
};

TSTATICASSERT( sizeof( AProximityRequirement ) == 0x54 );
