#include "pch.h"
#include "AControlComponentManager.h"
#include "AControlComponentData.h"
#include "Memory/AMemory.h"
#include "GameInterface/AGameState.h"
#include "Player/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058c580, CreateObject 0058cf20, CreateObjectInPlace 0058cf80
TDEFINE_CLASS( AControlComponentManager );

// $Barnyard: FUNCTION 0058ce50
AControlComponentManager::AControlComponentManager()
    : m_oPool( AMemory::GetAllocator( AMemory::POOL_Misc ), MAX_NUM_COMPONENTS )
{
	m_oGameStateListener.Connect( this, OnGameStateControllerEventCallback );
}

// $Barnyard: FUNCTION 0058cf00
// $Barnyard: FUNCTION 0058cda0
AControlComponentManager::~AControlComponentManager()
{
	// NOTE: the original listener disconnects itself in its destructor
	m_oGameStateListener.Disconnect();
}

// $Barnyard: FUNCTION 0058c6f0
AControlComponent* AControlComponentManager::CreateComponent( const TClass* a_pClass, AControlComponentData* a_pData )
{
	// NOTE: the original passes a_pClass->GetClassSize() to the pool's Malloc, which ignores the size
	void*              pMemory    = m_oPool.AllocateObject();
	AControlComponent* pComponent = TSTATICCAST( AControlComponent, a_pClass->CreateObjectInPlace( pMemory ) );

	pComponent->m_pData = a_pData;
	m_llComponents.PushBack( pComponent );

	return pComponent;
}

// $Barnyard: FUNCTION 0058c760
void AControlComponentManager::ReleaseComponent( AUnitComponent* a_pComp )
{
	AControlComponent* pComponent = TSTATICCAST( AControlComponent, a_pComp );

	pComponent->SetEnabled( TFALSE );
	pComponent->Remove();
	pComponent->~AControlComponent();

	m_oPool.FreeObject( pComponent );
}

// $Barnyard: FUNCTION 0058c7a0
TBOOL AControlComponentManager::ProcessInput( const TInputInterface::InputEvent* a_pEvent )
{
	TBOOL bHandled = TFALSE;

	for ( auto it = m_llComponents.Begin(); it != m_llComponents.End(); )
	{
		AControlComponent* pComponent = it++;

		// Only the component of the unit controlled by the player owning the event's device gets the input
		TBOOL bIsSourcePlayerUnit = TFALSE;

		APlayerSet* pPlayerSet = APlayerManager::GetSingleton()->GetCurrentPlayerSet();
		TINT        iSlot      = pPlayerSet->GetPlayerSlotIndexByDevice( a_pEvent->GetSource() );

		if ( pPlayerSet->GetTotalPlayerCount() != 0 )
			bIsSourcePlayerUnit = pPlayerSet->GetPlayerUnit( iSlot ) == pComponent->GetOwner();

		if ( bIsSourcePlayerUnit && pComponent->IsAIEnabled() )
		{
			AInputMapManager::GetSingleton()->PushInputMap( pComponent->GetData()->GetInputMap() );
			AInputMapManager::GetSingleton()->GetEventCommands( a_pEvent, m_oCommands );

			for ( TINT i = 0; i < m_oCommands.iNumCommands; i++ )
			{
				if ( pComponent->ProcessCommand( m_oCommands.aCommands[ i ], a_pEvent ) == TTRUE )
					bHandled = TTRUE;
			}

			AInputMapManager::GetSingleton()->PopInputMap();
		}
	}

	return bHandled;
}

// $Barnyard: FUNCTION 0058c8c0
void AControlComponentManager::OnGameStateControllerEvent( const AGameStateControllerEvent& a_rEvent )
{
	if ( a_rEvent.GetEventType() != AGameStateControllerEvent::Type_GameStateActivated )
		return;

	// Enable the components whose data lists the newly activated game state
	for ( auto it = m_llComponents.Begin(); it != m_llComponents.End(); )
	{
		AControlComponent* pComponent = it++;
		TBOOL              bEnable    = TFALSE;

		AControlComponentData* pData = pComponent->GetData();

		for ( auto itGameState = pData->m_vecGameStates.Begin(); itGameState != pData->m_vecGameStates.End(); itGameState++ )
		{
			if ( a_rEvent.GetGameState()->GetClass() == *itGameState )
			{
				bEnable = TTRUE;
				break;
			}
		}

		pComponent->SetEnabled( bEnable );
	}
}

// $Barnyard: FUNCTION 0058cc70
void AControlComponentManager::OnGameStateControllerEventCallback( void* a_pReceiver, const AGameStateControllerEvent& a_rEvent )
{
	TSTATICCAST( AControlComponentManager, a_pReceiver )->OnGameStateControllerEvent( a_rEvent );
}
