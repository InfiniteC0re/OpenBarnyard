#include "pch.h"
#include "AInventoryGroupComponentData.h"
#include "AInventoryComponentData.h"
#include "AInventoryGroupComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00622dc0, CreateObject 006232b0, CreateObjectInPlace 00623330
TDEFINE_CLASS( AInventoryGroupComponentData );

TPSTRING8_DECLARE( CategoryType );

// NOTE: inlined into CreateObject
AInventoryGroupComponentData::AInventoryGroupComponentData()
    : m_iNumCategories( 0 )
    , m_pCategories( TNULL )
{
}

// $Barnyard: FUNCTION 00622eb0
// $Barnyard: FUNCTION 00623690
AInventoryGroupComponentData::~AInventoryGroupComponentData()
{
	if ( m_pCategories )
		delete[] m_pCategories;
}

// $Barnyard: FUNCTION 006251d0
TBOOL AInventoryGroupComponentData::Create( const PBProperties* a_pProperties )
{
	for ( TUINT32 i = 0; i < a_pProperties->GetPropertyCount(); i++ )
	{
		if ( TPString8( a_pProperties->GetPropertyByIndex( i )->GetName().GetString() ) == TPS8( CategoryType ) )
			m_iNumCategories++;
	}

	m_pCategories = new AInventoryComponentData[ m_iNumCategories ];

	TINT iCategory = 0;

	for ( TUINT32 i = 0; i < a_pProperties->GetPropertyCount(); i++ )
	{
		const PBProperties::PBProperty* pProperty = a_pProperties->GetPropertyByIndex( i );

		if ( TPString8( pProperty->GetName().GetString() ) == TPS8( CategoryType ) )
			m_pCategories[ iCategory++ ].Create( pProperty->GetValue()->GetProperties() );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00625b10
void AInventoryGroupComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AInventoryGroupComponent* pComponent = new AInventoryGroupComponent();
	pComponent->SetOwner( a_pUnit );
	pComponent->Create( this );

	a_pUnit->AddComponent( pComponent );
}
