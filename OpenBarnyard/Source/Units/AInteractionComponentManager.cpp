#include "pch.h"
#include "AInteractionComponentManager.h"
#include "AInteractionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail registers RETURNS_ZERO as both create functions and an alignment of 1
TDEFINE_CLASS_NORUNTIME( AInteractionComponentManager );

// $Barnyard: FUNCTION 005a0f90
AInteractionComponentManager::AInteractionComponentManager()
{
}

// $Barnyard: FUNCTION 005a1010
// $Barnyard: FUNCTION 005a0710
AInteractionComponentManager::~AInteractionComponentManager()
{
}

// NOTE: this is the pool's NewObject (T2ObjectPool<AInteractionComponent, 200, 124, 4>)
// $Barnyard: FUNCTION 005a1b80
AInteractionComponent* AInteractionComponentManager::CreateComponent()
{
	return m_oPool.NewObject();
}

// $Barnyard: FUNCTION 005a1ac0
void AInteractionComponentManager::Update( TFLOAT a_fDeltaTime )
{
	if ( !AInteractionManager::GetSingleton()->IsEnabled() )
		return;

	// Collect the components whose units want an interaction first, assigning one
	// moves the component out of m_llComponents
	AInteractionComponent* apReadyComponents[ MAX_NUM_COMPONENTS ];
	TINT                   iNumReady = 0;

	for ( auto it = m_llComponents.Begin(); it != m_llComponents.End(); )
	{
		AInteractionComponent* pComponent = it++;

		if ( pComponent->UpdateTimers( a_fDeltaTime ) && iNumReady < MAX_NUM_COMPONENTS )
			apReadyComponents[ iNumReady++ ] = pComponent;
	}

	for ( TINT i = 0; i < iNumReady; i++ )
	{
		if ( apReadyComponents[ i ]->m_pSequence == TNULL )
			apReadyComponents[ i ]->TryAssignInteraction();
	}
}

// $Barnyard: FUNCTION 005a14b0
void AInteractionComponentManager::ReleaseComponent( AUnitComponent* a_pComp )
{
	AInteractionComponent* pComponent = TSTATICCAST( AInteractionComponent, a_pComp );

	pComponent->LeaveSequence( TFALSE );
	pComponent->T2DList<AInteractionComponent>::Node::Remove();
	pComponent->~AInteractionComponent();

	m_oPool.FreeObject( pComponent );
}
