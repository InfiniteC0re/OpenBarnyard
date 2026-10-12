#pragma once
#include "AInteractionActivity.h"

class AAnimationBreakpoint;

//-----------------------------------------------------------------------------
// Holds an animation breakpoint created from the properties of the activity.
// NOTE: the activity overrides nothing but Create, the breakpoint isn't used.
//-----------------------------------------------------------------------------
class AAnimationBreakpointActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AAnimationBreakpointActivity, AInteractionActivity );

public:
	AAnimationBreakpointActivity();
	~AAnimationBreakpointActivity();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

private:
	AAnimationBreakpoint* m_pBreakpoint; // 0x08
};

TSTATICASSERT( sizeof( AAnimationBreakpointActivity ) == 0xc );
