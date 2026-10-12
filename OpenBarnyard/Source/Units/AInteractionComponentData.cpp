#include "pch.h"
#include "AInteractionComponentData.h"
#include "AInteractionComponent.h"
#include "AInteractionComponentManager.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005a02f0, CreateObject 005a0570, CreateObjectInPlace 005a05d0
TDEFINE_CLASS( AInteractionComponentData );

// NOTE: inlined into CreateObject 005a0570 / CreateObjectInPlace 005a05d0
AInteractionComponentData::AInteractionComponentData()
    : m_bSpontaneousInteractions( TTRUE )
    , m_bAllowConversation( TTRUE )
{
}

// $Barnyard: FUNCTION 005a0630
// $Barnyard: FUNCTION 005a0320
AInteractionComponentData::~AInteractionComponentData()
{
}

// $Barnyard: FUNCTION 005a0330
TBOOL AInteractionComponentData::Create( const PBProperties* a_pProperties )
{
	m_bSpontaneousInteractions = TTRUE;

	// NOTE: retail doesn't check the value types
	const PBPropertyValue* pSpontaneous = a_pProperties->GetProperty( "spontaneousinteractions" );

	if ( pSpontaneous )
		m_bSpontaneousInteractions = pSpontaneous->GetRaw() != TNULL;

	const PBPropertyValue* pAllowConversation = a_pProperties->GetProperty( "allowconversation" );

	if ( pAllowConversation )
		m_bAllowConversation = pAllowConversation->GetRaw() != TNULL;

	return TTRUE;
}

// $Barnyard: FUNCTION 005a1e80
void AInteractionComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AInteractionComponentManager* pManager   = AInteractionComponentManager::GetSingleton();
	AInteractionComponent*        pComponent = pManager->CreateComponent();
	pManager->m_llComponents.PushBack( pComponent );

	// NOTE: both setters are inlined in retail
	pComponent->SetAllowNormalInteractions( m_bSpontaneousInteractions );
	pComponent->SetAllowConversation( m_bAllowConversation );

	a_pUnit->AddComponent( pComponent );
}
