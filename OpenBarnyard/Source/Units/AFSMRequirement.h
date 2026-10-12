#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit's AUnitBehaviour has the current drive "drive", runs the FSM "fsm"
// and is in its state "fsmstate" (only the set ones are checked).
// TClass 0x0078ab9c, vtable 0x0072a47c, size 0x58.
//-----------------------------------------------------------------------------
class AFSMRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AFSMRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	AFSMRequirement() = default;
	~AFSMRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strDrive; // 0x4c, "drive"
	Toshi::TPString8 m_strFSM;   // 0x50, "fsm"
	Toshi::TPString8 m_strState; // 0x54, "fsmstate"
};

TSTATICASSERT( sizeof( AFSMRequirement ) == 0x58 );
