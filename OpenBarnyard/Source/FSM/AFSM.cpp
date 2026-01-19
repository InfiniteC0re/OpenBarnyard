#include "pch.h"
#include "AFSM.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AFSM::AFSM( const Toshi::TPString8& a_strName )
    : m_strName( a_strName )
    , m_iNumArgs( 0 )
{
	TIMPLEMENT();
}

AFSM::~AFSM()
{
	TIMPLEMENT();
}

// $Barnyard: FUNCTION 005fccc0
void AFSM::ResetArguments()
{
	T2_FOREACH_ARRAY( m_apArgumentTypes, it )
	{
		m_apArgumentTypes[ it ] = PPropertyValue::TYPE_UNDEF;
	}
}
