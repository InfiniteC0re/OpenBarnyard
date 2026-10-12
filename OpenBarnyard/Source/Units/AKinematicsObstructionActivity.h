#pragma once
#include "AInteractionActivity.h"

class AKinematicsObstructionActivityMessage;

//-----------------------------------------------------------------------------
// Puts an obstruction around the units of the sequence so that other units
// steer around the interaction (see AKinematicsObstructionActivityMessage).
//-----------------------------------------------------------------------------
class AKinematicsObstructionActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( AKinematicsObstructionActivity, AInteractionActivity );

public:
	AKinematicsObstructionActivity();
	~AKinematicsObstructionActivity();

	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x18

private:
	TINT                                   m_iUnk08;   // 0x08, never used
	AKinematicsObstructionActivityMessage* m_pMessage; // 0x0c, message of the last sequence the activity started in
};

TSTATICASSERT( sizeof( AKinematicsObstructionActivity ) == 0x10 );
