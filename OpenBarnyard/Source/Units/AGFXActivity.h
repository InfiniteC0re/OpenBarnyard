#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Shows or hides the units of the category ("visible").
//-----------------------------------------------------------------------------
class AGFXActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AGFXActivity, AInteractionActivity );

	using VISIBILITY = TINT;
	enum VISIBILITY_ : VISIBILITY
	{
		VISIBILITY_UNCHANGED = 0, // "visible" isn't set
		VISIBILITY_SHOW      = 1,
		VISIBILITY_HIDE      = 2,
	};

public:
	AGFXActivity();
	~AGFXActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	VISIBILITY m_eVisibility; // 0x08, "visible"
};

TSTATICASSERT( sizeof( AGFXActivity ) == 0xc );
