#include "pch.h"
#include "AFSMContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005fe1c0
AFSMContext::AFSMContext( AFSMInstance* a_pInstance, const AFunctionCaller& a_rcCaller )
    : m_oCaller( a_rcCaller )
    , m_pInstance( a_pInstance )
    , m_bStay( TFALSE )
{
}

// $Barnyard: FUNCTION 005fdd20
void AFSMContext::ClearResult()
{
	m_oResult = PPropertyValue();
}

// $Barnyard: FUNCTION 005fe260
void AFSMContext::ClearTransition()
{
	m_oTransition = AFSMTransitionResult();
}

// $Barnyard: FUNCTION 005fe0e0
AFSMTransitionResult& AFSMTransitionResult::operator=( const AFSMTransitionResult& a_rcOther )
{
	pCallMachine = a_rcOther.pCallMachine;
	pState       = a_rcOther.pState;
	bDirect      = a_rcOther.bDirect;

	TINT iNumArguments = a_rcOther.oArguments.GetSize();
	oArguments.SetSize( iNumArguments );
	oArguments.Clear();

	for ( TINT i = 0; i < iNumArguments; i++ )
		oArguments.Push( *a_rcOther.oArguments.GetValue( i ) );

	return *this;
}
