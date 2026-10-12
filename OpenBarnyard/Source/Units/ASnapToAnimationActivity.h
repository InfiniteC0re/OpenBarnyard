#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Snaps the units to the "animation". With "waitforplayend" it waits until
// the unit of the category has finished playing it.
//-----------------------------------------------------------------------------
class ASnapToAnimationActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ASnapToAnimationActivity, AInteractionActivity );

public:
	ASnapToAnimationActivity();
	~ASnapToAnimationActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	TINT             m_iUnk08;          // 0x08
	Toshi::TPString8 m_strAnimation;    // 0x0c, "animation"
	Toshi::TPString8 m_strWaitAnim;     // 0x10, copy of m_strAnimation, see Update
	TBOOL            m_bWaitForPlayEnd; // 0x14, "waitforplayend"
};

TSTATICASSERT( sizeof( ASnapToAnimationActivity ) == 0x18 );
