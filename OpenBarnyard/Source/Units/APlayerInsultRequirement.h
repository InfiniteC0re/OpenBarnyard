#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The unit knows of an AInteractionPlayerInsultEvent from the last 3 minutes.
// TClass 0x0078addc, vtable 0x0072a8c0, size 0x58.
//-----------------------------------------------------------------------------
class APlayerInsultRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( APlayerInsultRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject
	APlayerInsultRequirement()
	    : m_iUnk1( 0 )
	    , m_iUnk2( 0 )
	    , m_bUnk3( TFALSE )
	{
		m_eCategory = AInteractionSequence::CATEGORY_PRIMARY;
	}

	~APlayerInsultRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TINT  m_iUnk1; // 0x4c, unused
	TINT  m_iUnk2; // 0x50, unused
	TBOOL m_bUnk3; // 0x54, unused
};

TSTATICASSERT( sizeof( APlayerInsultRequirement ) == 0x58 );
