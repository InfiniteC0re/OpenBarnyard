#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit's current FSM (AUnitBehaviour) is ("running" TRUE, the default) or
// isn't ("running" FALSE) the FSM "script".
// TClass 0x0078a9ec, vtable 0x0072a1c0, size 0x54.
//-----------------------------------------------------------------------------
class ABehaviourRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( ABehaviourRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	ABehaviourRequirement()
	    : m_bRunning( TTRUE )
	{
		m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;
	}

	~ABehaviourRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	Toshi::TPString8 m_strScript; // 0x4c, "script"
	TBOOL            m_bRunning;  // 0x50, "running"
};

TSTATICASSERT( sizeof( ABehaviourRequirement ) == 0x54 );
