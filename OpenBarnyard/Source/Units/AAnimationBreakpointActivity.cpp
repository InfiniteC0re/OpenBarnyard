#include "pch.h"
#include "AAnimationBreakpointActivity.h"
#include "Animation/ANamedAnimation.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b2f60, CreateObject 005b2fa0
// NOTE: GetClass 005b2f60, CreateObject 005b2fa0, CreateObjectInPlace 005b3010
TDEFINE_CLASS( AAnimationBreakpointActivity );

AAnimationBreakpointActivity::AAnimationBreakpointActivity()
    : m_pBreakpoint( TNULL )
{
}

// $Barnyard: FUNCTION 005b2f70
// $Barnyard: FUNCTION 005b3080
AAnimationBreakpointActivity::~AAnimationBreakpointActivity()
{
}

// $Barnyard: FUNCTION 005b2f80
TBOOL AAnimationBreakpointActivity::Create( const PBProperties* a_pProperties )
{
	m_pBreakpoint = ANamedAnimation::CreateBreakpointRuntime( a_pProperties, TFALSE );

	return AInteractionActivity::Create( a_pProperties );
}
