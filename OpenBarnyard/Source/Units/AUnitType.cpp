#include "pch.h"
#include "AUnitType.h"
#include "AUnit.h"
#include "AUnitManager.h"

#include <Toshi/TSystem.h>
#include <T2Locale/T2Locale.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00412910
AUnitType::AUnitType()
    : m_iScreenNameId( -1 )
    , m_pClass( TNULL )
    , m_iInstanceCount( 0 )
    , m_iUnk28( 0 )
{
}

// $Barnyard: FUNCTION 00412940
AUnitType::~AUnitType()
{
	m_llStaticData.DeleteAll();
}

// $Barnyard: FUNCTION 00412af0
TBOOL AUnitType::CreateFromProperties( const PBProperties* a_pProperties )
{
	const PBProperties* pUnitProperties = a_pProperties->FindProperties( "Unit" );

	// Name, stored lowercase
	TPString8 strName;
	TPString8 strPropertyName = TPS8D( "Name" );
	pUnitProperties->GetOptionalPropertyValue( strName, strPropertyName );

	TString8 strLowerName = strName.GetString8();
	strLowerName.MakeLower();
	m_strName = TPString8( strLowerName.GetString() );

	// Localised name
	m_iScreenNameId                    = -1;
	const PBPropertyValue* pScreenName = pUnitProperties->GetProperty( "ScreenName" );

	if ( pScreenName && pScreenName->GetType() == PBPropertyValue::Type::LocaleString )
		m_iScreenNameId = pScreenName->GetLocaleString().uiIndex;

	// Class to spawn. Classes that aren't linked into the game are loaded from Data/Units/<Class>.dll
	TPString8 strClassName;
	strPropertyName = TPS8D( "TClass" );

	if ( pUnitProperties->GetOptionalPropertyValue( strClassName, strPropertyName ) )
	{
		TClass* pClass = const_cast<TClass*>( TClass::Find( strClassName.GetString(), &TGetClass( AUnit ) ) );

		if ( !pClass )
		{
			TString8 strDllPath;
			strDllPath += "Data/Units/";
			strDllPath += strClassName.GetString8();
			strDllPath += ".dll";
			TKernelInterface::GetSingleton()->LoadInterface( strDllPath );

			pClass = const_cast<TClass*>( TClass::Find( strClassName.GetString(), &TGetClass( AUnit ) ) );
		}

		if ( pClass && !pClass->IsInitialized() )
			pClass->Initialize();

		m_pClass = pClass;
	}

	// Steer
	TPString8 strSteer;
	strPropertyName = TPS8D( "Steer" );

	if ( pUnitProperties->GetOptionalPropertyValue( strSteer, strPropertyName ) )
		m_strSteer = strSteer;

	LoadStaticData( a_pProperties );

	return TTRUE;
}

// $Barnyard: FUNCTION 00412a90
void AUnitType::ReindexInstances()
{
	TINT                     iIndex = 0;
	Toshi::TNodeList<AUnit>& rUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = rUnits.Begin(); it != rUnits.End(); it++ )
	{
		if ( !it->IsDying() && it->GetType() == this )
			it->SetUnitCountIdentifier( ++iIndex );
	}
}

// $Barnyard: FUNCTION 00412880
const TWCHAR* AUnitType::GetScreenName() const
{
	if ( m_iScreenNameId >= 0 )
		return T2Locale::GetSingleton()->GetString( m_iScreenNameId );

	return L"";
}

// $Barnyard: FUNCTION 004128a0
AUnitStaticData* AUnitType::FindStaticData( Toshi::TClass* a_pClass )
{
	for ( auto it = m_llStaticData.Begin(); it != m_llStaticData.End(); it++ )
	{
		if ( it->GetClass() == a_pClass )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 004129d0
void AUnitType::LoadStaticData( const PBProperties* a_pProperties )
{
	for ( TUINT32 i = 0; i < a_pProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pProperty = a_pProperties->GetPropertyByIndex( i );
		const TCHAR*                    szName    = pProperty->GetName().GetString();

		// Each sub-block named after an AUnitStaticData class becomes an instance of it.
		// "Animations" is an alias for AUnitModelStaticData.
		const TClass* pClass = TClass::Find( szName, &TGetClass( AUnitStaticData ) );

		if ( !pClass && TStringManager::String8CompareNoCase( szName, "Animations" ) == 0 )
			pClass = TClass::Find( "AUnitModelStaticData", &TGetClass( AUnitStaticData ) );

		if ( !pClass )
			continue;

		AUnitStaticData* pStaticData = TSTATICCAST( AUnitStaticData, pClass->CreateObject() );

		// NOTE: the original reads the value's payload without checking its type
		if ( pStaticData->Create( TSTATICCAST( const PBProperties, pProperty->GetValue()->GetRaw() ) ) )
			m_llStaticData.InsertTail( pStaticData );
		else
			delete pStaticData;
	}
}
