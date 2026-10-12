#pragma once
#include "AInteractionActivity.h"

//-----------------------------------------------------------------------------
// Thrown through TGlobalEmitter by ATriggerGuiActivity (listener list at
// 0x0078bc54).
//-----------------------------------------------------------------------------
struct ATriggerGuiEvent
{
	TINT   iUnk0;            // 0x00, always 0
	TINT   eBehaviour;       // 0x04, ATriggerGuiActivity::BEHAVIOUR
	AUnit* pUnit;            // 0x08
	TFLOAT fForceAnswerTime; // 0x0c
};

//-----------------------------------------------------------------------------
// Activates or deactivates ("behaviour") the GUI of the first unit of the
// category, see ATriggerGuiEvent.
//-----------------------------------------------------------------------------
class ATriggerGuiActivity
    : public AInteractionActivity
{
public:
	TDECLARE_CLASS( ATriggerGuiActivity, AInteractionActivity );

	using BEHAVIOUR = TINT;
	enum BEHAVIOUR_ : BEHAVIOUR
	{
		BEHAVIOUR_ACTIVATE   = 0, // "activate"
		BEHAVIOUR_DEACTIVATE = 1, // "deactivate"
	};

public:
	ATriggerGuiActivity();
	~ATriggerGuiActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c
	virtual RESULT Update( AInteractionSequence* a_pSequence ) OVERRIDE; // +0x1c

private:
	BEHAVIOUR m_eBehaviour;       // 0x08, "behaviour"
	TFLOAT    m_fForceAnswerTime; // 0x0c, "forceanswertime"
};

TSTATICASSERT( sizeof( ATriggerGuiActivity ) == 0x10 );
