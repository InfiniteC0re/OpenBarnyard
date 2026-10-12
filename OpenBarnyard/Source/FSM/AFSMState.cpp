#include "pch.h"
#include "AFSMState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005fd0d0
AFSMState::AFSMState()
    : m_iNumLocals( 0 )
{
}

// $Barnyard: FUNCTION 005fcee0
AFSMState::~AFSMState()
{
}

// $Barnyard: FUNCTION 005fcfc0
void AFSMState::SetName( const TPString8& a_rcName )
{
	m_strName = a_rcName;
}

// $Barnyard: FUNCTION 005fcd80
void AFSMState::AddInstruction( AFSMInstruction* a_pInstruction )
{
	m_llInstructions.Push( a_pInstruction );
}

// $Barnyard: FUNCTION 005fcd20
TINT AFSMState::FindLocalIndex( const TPString8& a_rcName ) const
{
	for ( TINT i = 0; i < m_iNumLocals; i++ )
	{
		if ( m_aLocalNames[ i ] == a_rcName )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005fcf70
TINT AFSMState::AddLocal( const TPString8& a_rcName )
{
	TINT iIndex             = m_iNumLocals;
	m_aLocalNames[ iIndex ] = a_rcName;
	m_iNumLocals++;

	return iIndex;
}
