#include "pch.h"
#include "AAnimationBreakpoint.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AAnimationBreakpoint );

AAnimationBreakpoint::AAnimationBreakpoint()
    : m_fTime( 0.0f )
{
}

AAnimationBreakpoint::~AAnimationBreakpoint()
{
}
