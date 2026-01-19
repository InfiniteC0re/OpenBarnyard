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
    , m_iNumVarTypes( 0 )
    , m_iNumVarNames( 0 )
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
	T2_FOREACH_ARRAY( m_aArgTypes, it )
	{
		m_aArgTypes[ it ] = PPropertyValue::TYPE_UNDEF;
	}
}

// $Barnyard: FUNCTION 005fd080
TINT AFSM::AddVariableName( const Toshi::TPString8& a_strName )
{
	TASSERT( m_iNumVarNames < MAX_NUM_ARGS );

	TINT iIDx = m_iNumVarNames++;
	m_aVarNames[ iIDx ] = a_strName;

	return iIDx;
}

// $Barnyard: FUNCTION 005fcd50
TINT AFSM::FindVariableNameIndex( const Toshi::TPString8& a_strName ) const
{
	for ( TINT i = 0; i < m_iNumVarNames; i++ )
		if ( m_aVarNames[ i ] == a_strName ) return i;

	return -1;
}

// $Barnyard: FUNCTION 005fccf0
TBOOL AFSM::SetVariableType( TINT a_iIndex, const Toshi::TClass* a_pTypeClass )
{
	const TClass* pOldTypeClass = m_aArgTypes[ a_iIndex ];
	m_aArgTypes[ a_iIndex ]     = a_pTypeClass;

	if ( pOldTypeClass != PPropertyValue::TYPE_UNDEF && a_pTypeClass != pOldTypeClass )
	{
		TASSERT( TFALSE && "Error: trying to change variable type!!!" );
		return TFALSE;
	}

	return TTRUE;
}
