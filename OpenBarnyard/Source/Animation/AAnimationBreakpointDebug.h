#pragma once
#include "AAnimationBreakpoint.h"

#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Holds a "Message". Does nothing when executed (the message was probably
// printed in debug builds).
//-----------------------------------------------------------------------------
class AAnimationBreakpointDebug
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointDebug, AAnimationBreakpoint );

public:
	AAnimationBreakpointDebug();
	~AAnimationBreakpointDebug();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE;               // +0x10
	virtual void  Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) OVERRIDE {} // +0x14, DOES_NOTHING (0054df00)

private:
	Toshi::TPString8 m_strMessage; // 0x08, "Message"
};

TSTATICASSERT( sizeof( AAnimationBreakpointDebug ) == 0xc );
