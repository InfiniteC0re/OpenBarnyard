#include "pch.h"
#include "AUnitProfileComponent.h"
#include "AUnitFlagSetsManager.h"
#include "AGeneralUnitComponentManager.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005d22c0, CreateObject 005d24b0, CreateObjectInPlace 005d2530
TDEFINE_CLASS( AUnitProfileComponent );

// NOTE: GetClass 005d22b0, CreateObject 005d2440, CreateObjectInPlace 005d23d0
TDEFINE_CLASS( AUnitProfileComponentData );

// NOTE: inlined into CreateObject 005d24b0 / CreateObjectInPlace 005d2530 / AUnitProfileComponentData::OnUnitCreate 005d2690
AUnitProfileComponent::AUnitProfileComponent()
    : m_uiRelationships( 0 )
    , m_uiInterests( 0 )
    , m_uiPlayerInterests( 0 )
{
	m_auiReputations[ 0 ] = 0;
	m_auiReputations[ 1 ] = 0;
}

// $Barnyard: FUNCTION 005d22e0
// $Barnyard: FUNCTION 005d22d0
AUnitProfileComponent::~AUnitProfileComponent()
{
}

// $Barnyard: FUNCTION 005d2300
AUnitComponentManager* AUnitProfileComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005d2310
void AUnitProfileComponent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( TINT( m_auiReputations[ 0 ] ) );
	a_pBuffer->WriteInt( TINT( m_auiReputations[ 1 ] ) );
	a_pBuffer->WriteInt( TINT( m_uiRelationships ) );
	a_pBuffer->WriteInt( TINT( m_uiInterests ) );
	a_pBuffer->WriteInt( TINT( m_uiPlayerInterests ) );
}

// $Barnyard: FUNCTION 005d2360
void AUnitProfileComponent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	// NOTE: retail uses the unsigned reader (0062e290), it reads the same 4 bytes as ReadInt
	a_pBuffer->ReadInt( *TREINTERPRETCAST( TINT*, &m_auiReputations[ 0 ] ) );
	a_pBuffer->ReadInt( *TREINTERPRETCAST( TINT*, &m_auiReputations[ 1 ] ) );
	a_pBuffer->ReadInt( *TREINTERPRETCAST( TINT*, &m_uiRelationships ) );
	a_pBuffer->ReadInt( *TREINTERPRETCAST( TINT*, &m_uiInterests ) );
	a_pBuffer->ReadInt( *TREINTERPRETCAST( TINT*, &m_uiPlayerInterests ) );
}

// NOTE: inlined into CreateObject 005d2440 / CreateObjectInPlace 005d23d0
AUnitProfileComponentData::AUnitProfileComponentData()
    : m_uiInterests( 0 )
{
}

// NOTE: the destructor is the shared one at 0061fa90
AUnitProfileComponentData::~AUnitProfileComponentData()
{
}

// $Barnyard: FUNCTION 005d25a0
TBOOL AUnitProfileComponentData::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pInterests = a_pProperties->GetProperty( "interests" );
	AUnitFlagSet*          pFlagSet   = AUnitFlagSetsManager::GetSingleton()->GetFlagSet( AUnitFlagSetsManager::FLAGSET_INTERESTS );

	if ( pInterests && pInterests->GetType() == PBPropertyValue::Type::Array && pInterests->GetArray() )
	{
		const PBPropertyValueArray* pArray = pInterests->GetArray();

		for ( TUINT i = 0; i < pArray->GetSize(); i++ )
		{
			TINT iFlag = pFlagSet->GetFlagIndex( pArray->GetValue( i )->GetTPString8() );
			m_uiInterests |= 1U << ( iFlag & 31 );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005d2690
void AUnitProfileComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AUnitProfileComponent* pComponent = new AUnitProfileComponent();
	pComponent->m_uiInterests         = m_uiInterests;

	a_pUnit->AddComponent( pComponent );
}
