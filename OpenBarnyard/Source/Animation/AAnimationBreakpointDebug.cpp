#include "pch.h"
#include "AAnimationBreakpointDebug.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005846b0, CreateObject 005846f0, CreateObjectInPlace 00584760 (the constructor is inlined in both)
TDEFINE_CLASS( AAnimationBreakpointDebug );

AAnimationBreakpointDebug::AAnimationBreakpointDebug()
{
}

// $Barnyard: FUNCTION 005847f0
// $Barnyard: FUNCTION 005847d0
AAnimationBreakpointDebug::~AAnimationBreakpointDebug()
{
}

TPSTRING8_DECLARE( Message );

// $Barnyard: FUNCTION 005846c0
TBOOL AAnimationBreakpointDebug::Create( const PBProperties* a_pProperties )
{
	TBOOL bResult = AAnimationBreakpoint::Create( a_pProperties );
	a_pProperties->GetOptionalPropertyValue( m_strMessage, TPS8( Message ) );

	return bResult;
}
