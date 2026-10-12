#include "pch.h"
#include "AControlComponentData.h"
#include "AControlComponent.h"
#include "AControlComponentManager.h"
#include "AUnit.h"
#include "Input/AInputMapManager.h"
#include "GameInterface/AGameState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058c560, CreateObject 0058cc90, CreateObjectInPlace 0058cd10
TDEFINE_CLASS( AControlComponentData );

// NOTE: inlined into CreateObject 0058cc90 / CreateObjectInPlace 0058cd10
AControlComponentData::AControlComponentData()
{
}

// $Barnyard: FUNCTION 0058cd80
// $Barnyard: FUNCTION 0058c9e0
AControlComponentData::~AControlComponentData()
{
}

// $Barnyard: FUNCTION 0058ca50
TBOOL AControlComponentData::Create( const PBProperties* a_pProperties )
{
	TPString8 strInputMap;
	a_pProperties->GetOptionalPropertyValue( strInputMap, TPS8D( "inputmap" ) );
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( strInputMap );

	a_pProperties->GetOptionalPropertyValue( m_strType, TPS8D( "type" ) );

	const PBPropertyValue* pGameStates = a_pProperties->GetProperty( "gamestates" );

	if ( pGameStates )
	{
		const PBPropertyValueArray* pArray       = pGameStates->GetArray();
		TINT                        iNumElements = pArray->GetSize();

		for ( TINT i = 0; i < iNumElements; i++ )
		{
			TPString8 strGameState = pArray->GetValue( i )->GetTPString8();
			m_vecGameStates.PushBack( TClass::Find( strGameState.GetString(), &TGetClass( AGameState ) ) );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0058cc10
void AControlComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	const TClass* pClass = TClass::Find( m_strType.GetString(), &TGetClass( AControlComponent ) );

	AControlComponent* pComponent = AControlComponentManager::GetSingleton()->CreateComponent( pClass, this );
	a_pUnit->AddComponent( pComponent );
	pComponent->SetEnabled( TTRUE );
}
