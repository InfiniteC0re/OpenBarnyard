#include "pch.h"
#include "AAnimationBreakpointCallback.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00584450, CreateObject 005844f0, CreateObjectInPlace 00584550
TDEFINE_CLASS( AAnimationBreakpointCallback );

// $Barnyard: FUNCTION 00584490
AAnimationBreakpointCallback::AAnimationBreakpointCallback()
    : m_oEmitter( this )
{
}

// $Barnyard: FUNCTION 00584590
// $Barnyard: FUNCTION 005845b0
AAnimationBreakpointCallback::~AAnimationBreakpointCallback()
{
}

TPSTRING8_DECLARE( Name );

// $Barnyard: FUNCTION 00584460
TBOOL AAnimationBreakpointCallback::Create( const PBProperties* a_pProperties )
{
	TBOOL bResult = AAnimationBreakpoint::Create( a_pProperties );
	a_pProperties->GetOptionalPropertyValue( m_strName, TPS8( Name ) );

	return bResult;
}

// $Barnyard: FUNCTION 00584630
void AAnimationBreakpointCallback::Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime )
{
	Event oEvent;
	oEvent.strName = m_strName;
	oEvent.pObject = a_pObject;

	m_oEmitter.Throw( &oEvent );
}
