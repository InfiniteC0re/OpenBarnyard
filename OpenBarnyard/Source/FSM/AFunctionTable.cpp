#include "pch.h"
#include "AFunctionTable.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AFunction::~AFunction() = default;

// $Barnyard: FUNCTION 005c96d0
AFunctionTable::AFunctionTable( Toshi::TClass* a_pClass, AFunctionTable* a_pParentTable )
    : m_pClass( a_pClass )
    , m_pParentTable( a_pParentTable )
    , m_aFunctions( 10, 0 )
    , m_fnFallback( TNULL )
{
}

// $Barnyard: FUNCTION 005c9750
// $Barnyard: FUNCTION 005c9890
AFunctionTable::~AFunctionTable() = default;

// $Barnyard: FUNCTION 005c9990
void AFunctionTable::Register( void* a_fnFallback )
{
	Toshi::TString8 strClassName( m_pClass->GetName() );
	strClassName.MakeLower();
	m_strClassNameLower = strClassName.GetString();

	if ( m_pParentTable != TNULL )
	{
		const TINT iNumParentFunctions = m_pParentTable->m_aFunctions.Size();

		if ( m_aFunctions.Capacity() < iNumParentFunctions )
		{
			m_aFunctions.AccessContainer()->Resize( iNumParentFunctions );
		}

		// Make sure it's empty
		m_aFunctions.AccessContainer()->SetSize( 0 );

		for ( TINT i = 0; i < iNumParentFunctions; i++ )
		{
			AFunction* pFunction = m_pParentTable->m_aFunctions[ i ]->Clone();
			m_aFunctions.Push( pFunction );
		}
	}

	m_fnFallback = a_fnFallback;

	if ( m_fnFallback == TNULL && m_pParentTable != TNULL )
	{
		m_fnFallback = m_pParentTable->m_fnFallback;
	}

	GetRegisteredTables().InsertTail( this );
}

Toshi::TNodeList<AFunctionTable>& AFunctionTable::GetRegisteredTables()
{
	static Toshi::TNodeList<AFunctionTable>* s_pFunctionTables = new Toshi::TNodeList<AFunctionTable>();
	return *s_pFunctionTables;
}

// $Barnyard: FUNCTION 005c98b0
void AFunctionTable::Unregister()
{
	const TINT iNumFunctions = m_aFunctions.Size();

	for ( TINT i = 0; i < iNumFunctions; i++ )
	{
		AFunction* pFunction = m_aFunctions[ i ];

		if ( pFunction != TNULL )
		{
			delete pFunction;
		}
	}

	m_aFunctions.AccessContainer()->Resize( 0 );

	if ( IsLinked() )
	{
		GetRegisteredTables().Remove( this );
	}

	m_strClassNameLower = Toshi::TPString8();
}

// $Barnyard: FUNCTION 005c9b70
void AFunctionTable::AddFunction( AFunction* a_pFunction )
{
	m_aFunctions.Push( a_pFunction );
}

// T2SortedVector/FunctionContainer methods:
// $Barnyard: FUNCTION 005c9410
// $Barnyard: FUNCTION 005c94f0
// $Barnyard: FUNCTION 0066fcd0
