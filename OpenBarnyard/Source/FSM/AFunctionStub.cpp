#include "pch.h"
#include "AFunctionStub.h"
#include "Units/AUnitComponent.h"
#include "Units/ACharacterUnit.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Plugins/PPropertyParser/PPropertyValueArray.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AFunctionStub::AFunctionStub( const TCHAR* a_szName, const TCHAR* a_szTableName, const TClass* a_pReturnType, std::initializer_list<const TClass*> a_lstArgTypes )
    : AFunction( a_szName )
    , m_szTableName( a_szTableName )
    , m_pReturnType( a_pReturnType )
    , m_iNumArgs( 0 )
    , m_bLogged( TFALSE )
{
	TASSERT( a_lstArgTypes.size() <= MAX_NUM_ARGS );

	for ( const TClass* pType : a_lstArgTypes )
		m_apArgTypes[ m_iNumArgs++ ] = pType;
}

AFunctionStub::AFunctionStub( const AFunctionStub& a_rOther )
    : AFunction( a_rOther )
    , m_szTableName( a_rOther.m_szTableName )
    , m_pReturnType( a_rOther.m_pReturnType )
    , m_iNumArgs( a_rOther.m_iNumArgs )
    , m_bLogged( TFALSE )
{
	for ( TINT i = 0; i < m_iNumArgs; i++ )
		m_apArgTypes[ i ] = a_rOther.m_apArgTypes[ i ];
}

AFunctionStub::~AFunctionStub()
{
}

AFunction* AFunctionStub::Clone() const
{
	return new AFunctionStub( *this );
}

void AFunctionStub::Call( TObject* a_pObject, PPropertyValue* a_pResult, const PPropertyValueArray* a_pArgs ) const
{
	if ( !m_bLogged )
	{
		TWARN( "FSM native %s::%s isn't implemented\n", m_szTableName, GetName()->GetString() );
		m_bLogged = TTRUE;
	}

	if ( !a_pResult )
		return;

	// Neutral value of the return type
	if ( m_pReturnType == PPropertyValue::TYPE_BOOL )
		a_pResult->SetBoolean( TFALSE );
	else if ( m_pReturnType == PPropertyValue::TYPE_INT )
		a_pResult->SetInteger( 0 );
	else if ( m_pReturnType == PPropertyValue::TYPE_UINT32 )
		a_pResult->SetUINT32( 0 );
	else if ( m_pReturnType == PPropertyValue::TYPE_FLOAT )
		a_pResult->SetFloat( 0.0f );
	else if ( m_pReturnType == PPropertyValue::TYPE_TPSTRING8 )
		a_pResult->SetTPString8( TPString8() );
	else if ( m_pReturnType != PPropertyValue::TYPE_UNDEF )
		a_pResult->SetTObject( m_pReturnType, TNULL );
}

void AFunctionStub::GetSignature( const TClass** a_ppReturnType, TArray<const TClass*>& a_rArgTypes ) const
{
	*a_ppReturnType = m_pReturnType;

	for ( TINT i = 0; i < m_iNumArgs; i++ )
		a_rArgTypes.Push( m_apArgTypes[ i ] );
}

//-----------------------------------------------------------------------------
// Function tables of the classes that aren't reconstructed yet. The names and
// signatures are the ones of the retail tables (InitialiseClass addresses below).
// NOTE: the classes only exist for the names of the tables, nothing creates them.
//-----------------------------------------------------------------------------

#define STUB_CLASS( NAME ) \
	static TClass s_oStubClass_##NAME( #NAME, TNULL, TNULL, TNULL, TNULL, TNULL, 0, 1, 0, 4 )

#define STUB_TABLE( NAME, PARENT_TABLE ) \
	static AFunctionTable s_oStubTable_##NAME( &s_oStubClass_##NAME, PARENT_TABLE )

static TBOOL s_bStubTablesRegistered = TFALSE;

void AFunctionStub::RegisterStubTables()
{
	if ( s_bStubTablesRegistered )
		return;

	s_bStubTablesRegistered = TTRUE;

	// No stub tables left (ASoundComponent is reconstructed in Units/ASoundComponent.cpp)
}

void AFunctionStub::UnregisterStubTables()
{
	if ( !s_bStubTablesRegistered )
		return;

	s_bStubTablesRegistered = TFALSE;
}
