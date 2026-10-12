#include "pch.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "AUnit.h"
#include "AUnitManager.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0040e290, CreateObject 0040e780, CreateObjectInPlace 0040e800 (the constructor is inlined in both)
TDEFINE_CLASS( AUnitFactory );

// $Barnyard: FUNCTION 0040e620
AUnitFactory::AUnitFactory()
{
}

// $Barnyard: FUNCTION 0040e880
// $Barnyard: FUNCTION 0040e650
AUnitFactory::~AUnitFactory()
{
	m_llTypes.DeleteAll();
}

// $Barnyard: FUNCTION 0040e6b0
TBOOL AUnitFactory::LoadUnitTypes( const TCHAR* a_szFilePath )
{
	TTRB oTRB;

	if ( oTRB.Load( a_szFilePath ) != TTRB::ERROR_OK )
		return TFALSE;

	const PBProperties* pTypes = TSTATICCAST( const PBProperties, oTRB.GetSymbolAddress( "Main" ) );
	oTRB.DeleteSymbolTable();

	for ( TUINT32 i = 0; i < pTypes->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pType = pTypes->GetPropertyByIndex( i );
		RegisterType( pType->GetName().GetString(), TSTATICCAST( const PBProperties, pType->GetValue()->GetRaw() ) );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0040e4f0
AUnitType* AUnitFactory::RegisterType( const TCHAR* a_szName, const PBProperties* a_pProperties )
{
	TString8 strName( a_szName );
	strName.MakeLower();

	// Don't register the same name twice
	if ( Find( TPString8( strName.GetString() ) ) )
		return TNULL;

	AUnitType* pType = new AUnitType();
	pType->CreateFromProperties( a_pProperties );
	m_llTypes.InsertTail( pType );

	return pType;
}

// $Barnyard: FUNCTION 0040e370
AUnitType* AUnitFactory::Find( const TPString8& a_rcName )
{
	// Names are stored pooled and lowercase, so match the same way
	TPString8 strName = a_rcName;

	if ( !a_rcName.GetString8().IsAllLowerCase() )
	{
		TString8 strLower = a_rcName.GetString8();
		strLower.MakeLower();
		strName = TPString8( strLower.GetString() );
	}

	for ( auto it = m_llTypes.Begin(); it != m_llTypes.End(); it++ )
	{
		if ( it->GetName() == strName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 0040e2a0
AUnit* AUnitFactory::CreateInstance( AUnitType* a_pType )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: safety net for the types whose class isn't reconstructed yet : they have no TClass
	if ( !a_pType->m_pClass )
		return TNULL;
#endif

	AUnit* pUnit = TSTATICCAST( AUnit, a_pType->m_pClass->CreateObject() );

	if ( pUnit )
	{
		pUnit->m_pType = a_pType;
		a_pType->m_iInstanceCount++;

		if ( pUnit->DoesHaveUnitCountIdentifier() )
			pUnit->SetUnitCountIdentifier( pUnit->m_pType->m_iInstanceCount );
	}

	return pUnit;
}

// $Barnyard: FUNCTION 0040e2e0
void AUnitFactory::ReleaseInstance( AUnit* a_pUnit )
{
	if ( !a_pUnit )
		return;

	AUnitType* pType = a_pUnit->GetType();
	pType->m_iInstanceCount--;

	if ( a_pUnit->DoesHaveUnitCountIdentifier() && !AUnitManager::GetSingleton()->IsDying() )
		pType->ReindexInstances();
}
