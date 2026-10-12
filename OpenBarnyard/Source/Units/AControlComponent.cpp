#include "pch.h"
#include "AControlComponent.h"
#include "AControlComponentManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( AControlComponent );

// $Barnyard: FUNCTION 0058c600
AControlComponent::AControlComponent()
{
	// Control components start disabled until a matching game state activates them
	m_bAIEnabled = TFALSE;
}

// $Barnyard: FUNCTION 0058c620
// $Barnyard: FUNCTION 0058c590
AControlComponent::~AControlComponent()
{
}

// $Barnyard: FUNCTION 0058c640
AUnitComponentManager* AControlComponent::GetManager()
{
	return AControlComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 0058c5a0
void AControlComponent::SetEnabled( TBOOL a_bEnabled )
{
	if ( a_bEnabled != m_bAIEnabled )
	{
		m_bAIEnabled = a_bEnabled;

		if ( !a_bEnabled )
			OnDisable();
		else
			OnEnable();
	}
}
