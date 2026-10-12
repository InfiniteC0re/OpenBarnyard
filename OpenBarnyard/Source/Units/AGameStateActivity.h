#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Waits until the game state "state1" becomes the current one and then until
// "state2" does (state names are class names, e.g. "ASimState").
//-----------------------------------------------------------------------------
class AGameStateActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AGameStateActivity, AInteractionActivity );

public:
	AGameStateActivity();
	~AGameStateActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE;  // +0x0c
	virtual void   GetDataSize( TINT* a_pSize ) OVERRIDE;                 // +0x10
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE;  // +0x1c

private:
	Toshi::TPString8 m_strState1; // 0x08, "state1"
	Toshi::TPString8 m_strState2; // 0x0c, "state2"
	TINT             m_iUnk10;    // 0x10, never used
};

TSTATICASSERT( sizeof( AGameStateActivity ) == 0x14 );
