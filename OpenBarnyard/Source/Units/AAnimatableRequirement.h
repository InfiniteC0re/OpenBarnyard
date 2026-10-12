#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit is an AAnimatedUnit whose animation set has the animation "animation".
// TClass 0x0078a95c, vtable 0x0072a10c, size 0x50.
//-----------------------------------------------------------------------------
class AAnimatableRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AAnimatableRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AAnimatableRequirement() = default;
	~AAnimatableRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strAnimation; // 0x4c, "animation"
};

TSTATICASSERT( sizeof( AAnimatableRequirement ) == 0x50 );
