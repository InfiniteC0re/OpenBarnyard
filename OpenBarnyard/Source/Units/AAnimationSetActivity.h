#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Changes the animation set of the units of the category to "animationset".
//-----------------------------------------------------------------------------
class AAnimationSetActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AAnimationSetActivity, AInteractionActivity );

public:
	AAnimationSetActivity();
	~AAnimationSetActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strAnimationSet; // 0x08, "animationset"
};

TSTATICASSERT( sizeof( AAnimationSetActivity ) == 0xc );
