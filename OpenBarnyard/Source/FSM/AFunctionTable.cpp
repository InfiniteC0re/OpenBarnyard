#include "pch.h"
#include "AFunctionTable.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 005c90c0
AFunction::AFunction( const TCHAR* a_szName )
{
	m_pNameLower = new TPString8();

	TString8 strName( a_szName );
	strName.MakeLower();
	*m_pNameLower = TPString8( strName.GetString() );
}

// $Barnyard: FUNCTION 005c91c0
AFunction::AFunction( const AFunction& a_rOther )
{
	m_pNameLower  = new TPString8();
	*m_pNameLower = *a_rOther.m_pNameLower;
}

// $Barnyard: FUNCTION 005c92b0
// $Barnyard: FUNCTION 005c9220
AFunction::~AFunction()
{
	if ( m_pNameLower )
		delete m_pNameLower;
}

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

// NOTE: retail uses a function-local static list without a destructor (no atexit), allocate it to match that
// $Barnyard: FUNCTION 005c97d0
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

// $Barnyard: FUNCTION 005c9340
TObject* AFunctionTable::GetFallbackObject( const AFunctionCaller* a_pCaller ) const
{
	using FallbackFn = TObject*(__stdcall*)( const AFunctionTable*, const AFunctionCaller* );

	if ( m_fnFallback )
		return TREINTERPRETCAST( FallbackFn, m_fnFallback )( this, a_pCaller );

	return TNULL;
}

// NOTE: same as the engine TPString8 comparison at 006bc110
static TINT ComparePooledStrings( const TPString8& a_rcLeft, const TPString8& a_rcRight )
{
	if ( a_rcLeft.GetPooledString() > a_rcRight.GetPooledString() )
		return 1;

	return -( a_rcLeft.GetPooledString() < a_rcRight.GetPooledString() );
}

// Finds the first function named a_rcName (binary search, the functions are sorted by the pooled string).
// Returns the index of the function or -1 (TArray end iterator) if there's no such function.
// $Barnyard: FUNCTION 005c95e0
static TINT FindFirstFunction( const AFunctionTable::FunctionList& a_rcFunctions, const TPString8& a_rcName )
{
	TINT iStart = 0;
	TINT iEnd   = a_rcFunctions.Size() - 1;

	while ( iStart <= iEnd )
	{
		TINT iMiddle = ( iStart + iEnd ) / 2;
		TINT iCmp    = -ComparePooledStrings( *a_rcFunctions[ iMiddle ]->GetName(), a_rcName );

		if ( iCmp < 0 )
		{
			iEnd = iMiddle - 1;
		}
		else if ( iCmp > 0 )
		{
			iStart = iMiddle + 1;
		}
		else
		{
			// Go back to the first function of the name
			TINT i = iMiddle - 1;

			for ( ; i >= iStart; i-- )
			{
				if ( ComparePooledStrings( *a_rcFunctions[ i ]->GetName(), a_rcName ) != 0 )
					break;
			}

			return i + 1;
		}
	}

	return -1;
}

// $Barnyard: FUNCTION 005c9680
AFunction* AFunctionTable::FindFunction( const TPString8& a_rcName ) const
{
	TINT iIndex = FindFirstFunction( m_aFunctions, a_rcName );
	return ( iIndex == -1 ) ? TNULL : m_aFunctions[ iIndex ];
}

// Debug leftover: gets the signatures of all of the functions and doesn't use them
// $Barnyard: FUNCTION 005c9360
void AFunctionTable::BuildSignatures() const
{
	const TINT iNumFunctions = m_aFunctions.Size();

	for ( TINT i = 0; i < iNumFunctions; i++ )
	{
		const TClass*         pReturnType;
		TArray<const TClass*> vecArgTypes( 10, 0 );
		m_aFunctions[ i ]->GetSignature( &pReturnType, vecArgTypes );
	}
}

// Debug leftover: walks the registered tables without doing anything
// $Barnyard: FUNCTION 005c9810
void AFunctionTable::WalkRegisteredTables()
{
	TNodeList<AFunctionTable>& rTables = GetRegisteredTables();

	for ( auto it = rTables.Begin(); it != rTables.End(); it++ )
		;
}

// T2SortedVector/FunctionContainer methods:
// $Barnyard: FUNCTION 005c9410
// $Barnyard: FUNCTION 005c94f0
// $Barnyard: FUNCTION 0066fcd0

// $Barnyard: FUNCTION 005c8e60
TBOOL AFunctionRef::Find( const TPString8& a_rcTable, const TPString8& a_rcFunction )
{
	TNodeList<AFunctionTable>& rTables = AFunctionTable::GetRegisteredTables();

	for ( auto it = rTables.Begin(); it != rTables.End(); it++ )
	{
		if ( it->GetClassNameLower() == a_rcTable )
		{
			AFunction* pFoundFunction = it->FindFunction( a_rcFunction );

			if ( !pFoundFunction )
			{
				it->BuildSignatures();
				return TFALSE;
			}

			pTable    = it;
			pFunction = pFoundFunction;
			return TTRUE;
		}
	}

	AFunctionTable::WalkRegisteredTables();
	return TFALSE;
}

// $Barnyard: FUNCTION 005c9320
static TBOOL __stdcall InvokeFunction( AFunction* a_pFunction, TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs )
{
	a_pFunction->Call( a_pObject, a_pResult, a_pArgs );
	return TTRUE;
}

// $Barnyard: FUNCTION 005c8e10
TBOOL AFunctionRef::Call( const AFunctionCaller& a_rcCaller, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const
{
	if ( !pTable || !pFunction )
		return TFALSE;

	TObject* pObject = pTable->GetFallbackObject( &a_rcCaller );

	// Methods need an object
	if ( !pObject && pFunction->IsMethod() )
		return TFALSE;

	return InvokeFunction( pFunction, pObject, a_pResult, a_pArgs );
}

// $Barnyard: FUNCTION 005c8d40
TString8 AFunctionRef::GetName() const
{
	const TCHAR* szTable    = pTable ? pTable->GetClassNameLower().GetString() : "UnknownTable";
	const TCHAR* szFunction = pFunction ? pFunction->GetName()->GetString() : "UnknownFunction";

	TString8 strName;
	strName.Format( "%s::%s", szTable, szFunction );

	return strName;
}

// $Barnyard: FUNCTION 005c8d10
TBOOL AFunctionRef::GetSignature( const TClass** a_ppReturnType, TArray<const TClass*>& a_rArgTypes ) const
{
	if ( !pFunction )
		return TFALSE;

	pFunction->GetSignature( a_ppReturnType, a_rArgTypes );
	return TTRUE;
}
