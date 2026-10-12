#pragma once
#include "AInteractionRequirement.h"

//-----------------------------------------------------------------------------
// The persistent latch "key" has the value "keyval".
// TClass 0x0078ab0c, vtable 0x0072a350, size 0x60.
//-----------------------------------------------------------------------------
class AEventRequirement
    : public AInteractionRequirement
{
public:
	TDECLARE_CLASS( AEventRequirement, AInteractionRequirement );

public:
	// NOTE: inlined into CreateObject, the members aren't initialised
	AEventRequirement() = default;
	~AEventRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;           // +0x0c
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) OVERRIDE; // +0x1c

private:
	TCHAR m_szKey[ 16 ]; // 0x4c, "key"
	TINT  m_iKeyValue;   // 0x5c, "keyval"
};

TSTATICASSERT( sizeof( AEventRequirement ) == 0x60 );
