#include "pch.h"
#include "AFSM.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005fcff0
AFSM::AFSM( const Toshi::TPString8& a_strName )
    : m_strName( a_strName )
    , m_pExitStartState( TNULL )
    , m_pExitEndState( TNULL )
    , m_pCommonState( TNULL )
    , m_iNumArguments( 0 )
    , m_iNumVarNames( 0 )
{
}

// $Barnyard: FUNCTION 005fd170
AFSM::~AFSM()
{
	// $Barnyard: FUNCTION 005fd140
	while ( !m_llStates.IsEmpty() )
	{
		AFSMState* pState = m_llStates.Head();
		pState->Remove();
		delete pState;
	}
}

// $Barnyard: FUNCTION 005fcdb0
void AFSM::AddState( AFSMState* a_pState, TBOOL a_bDefault, TBOOL a_bExitStart, TBOOL a_bExitEnd )
{
	if ( a_bDefault )
		m_llStates.InsertHead( a_pState );
	else
		m_llStates.InsertTail( a_pState );

	if ( a_bExitStart )
		m_pExitStartState = a_pState;

	if ( a_bExitEnd )
		m_pExitEndState = a_pState;
}

// $Barnyard: FUNCTION 005fce00
void AFSM::SetCommonState( AFSMState* a_pState )
{
	m_llStates.InsertTail( a_pState );
	m_pCommonState = a_pState;
}

// $Barnyard: FUNCTION 005fce20
void AFSM::Reset()
{
	T2_FOREACH( m_llStates, it )
	{
		it->GetInstructions().Reset();
	}

	// NOTE: these states are in the list too, retail resets them twice
	if ( m_pExitStartState )
		m_pExitStartState->GetInstructions().Reset();

	if ( m_pExitEndState )
		m_pExitEndState->GetInstructions().Reset();

	if ( m_pCommonState )
		m_pCommonState->GetInstructions().Reset();
}

// $Barnyard: FUNCTION 005fccc0
void AFSM::ResetArguments()
{
	T2_FOREACH_ARRAY( m_aVarTypes, it )
	{
		m_aVarTypes[ it ] = PPropertyValue::TYPE_UNDEF;
	}
}

// $Barnyard: FUNCTION 005fd080
TINT AFSM::AddVariableName( const Toshi::TPString8& a_strName )
{
	TINT iIndex           = m_iNumVarNames;
	m_aVarNames[ iIndex ] = a_strName;
	m_iNumVarNames++;

	return iIndex;
}

// $Barnyard: FUNCTION 005fcd50
TINT AFSM::FindVariableNameIndex( const Toshi::TPString8& a_strName ) const
{
	for ( TINT i = 0; i < m_iNumVarNames; i++ )
	{
		if ( m_aVarNames[ i ] == a_strName )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005fcce0
const TClass* AFSM::GetVariableType( TINT a_iIndex ) const
{
	return m_aVarTypes[ a_iIndex ];
}

// $Barnyard: FUNCTION 005fccf0
TBOOL AFSM::SetVariableType( TINT a_iIndex, const Toshi::TClass* a_pTypeClass )
{
	const TClass* pOldTypeClass = m_aVarTypes[ a_iIndex ];
	m_aVarTypes[ a_iIndex ]     = a_pTypeClass;

	if ( pOldTypeClass != PPropertyValue::TYPE_UNDEF )
		return a_pTypeClass == pOldTypeClass;

	return TTRUE;
}
