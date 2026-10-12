#include "pch.h"
#include "AInventoryComponentData.h"
#include "AInventoryComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00622db0, CreateObject 006258a0, CreateObjectInPlace 00624380
TDEFINE_CLASS( AInventoryComponentData );

TPSTRING8_DECLARE( MaxItems );
TPSTRING8_DECLARE( CategoryName );
TPSTRING8_DECLARE( ScreenName );
TPSTRING8_DECLARE( Item );

// $Barnyard: FUNCTION 00624360
AInventoryComponentData::AInventoryComponentData()
    : m_iMaxItems( 5 )
    , m_iScreenName( -1 )
{
}

// $Barnyard: FUNCTION 006243f0
// $Barnyard: FUNCTION 00625340
AInventoryComponentData::~AInventoryComponentData()
{
	while ( !m_llItems.IsEmpty() )
	{
		AInventoryItemData* pItemData = m_llItems.PopFront();

		if ( pItemData )
			delete pItemData;
	}

	m_llItems.Clear();
}

// $Barnyard: FUNCTION 006244a0
TBOOL AInventoryComponentData::Create( const PBProperties* a_pProperties )
{
	m_iMaxItems = 5;
	a_pProperties->GetOptionalPropertyValue( m_iMaxItems, TPS8( MaxItems ) );
	a_pProperties->GetOptionalPropertyValue( m_strCategoryName, TPS8( CategoryName ) );

	PBPropertyLocaleString oScreenName = TUINT( -1 );

	if ( const PBPropertyValue* pScreenName = a_pProperties->GetProperty( TPS8( ScreenName ) ) )
	{
		if ( pScreenName->GetType() == PBPropertyValue::Type::LocaleString )
			oScreenName = pScreenName->GetLocaleString();
	}

	m_iScreenName = TINT( oScreenName.uiIndex );

	// The starting items
	for ( TUINT32 i = 0; i < a_pProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pProperty = a_pProperties->GetPropertyByIndex( i );

		if ( TPString8( pProperty->GetName().GetString() ) == TPS8( Item ) )
		{
			AInventoryItemData* pItemData = new AInventoryItemData();
			m_llItems.PushBack( pItemData );
			pItemData->Create( pProperty->GetValue()->GetProperties() );
		}
	}

	return AUnitStaticData::Create( a_pProperties );
}

// $Barnyard: FUNCTION 00625a70
void AInventoryComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AInventoryComponent* pComponent = new AInventoryComponent();
	pComponent->Create( this, a_pUnit );

	a_pUnit->AddComponent( pComponent );
}
