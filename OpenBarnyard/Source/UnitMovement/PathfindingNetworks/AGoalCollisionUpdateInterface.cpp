#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 0054f640
// $Barnyard: FUNCTION 00414fb0
AGoalCollisionUpdateInterface::~AGoalCollisionUpdateInterface()
{
	AInteriorNetworkManager::GetSingleton()->RemoveGoalInterface( this );
}
