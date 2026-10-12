#include "pch.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateGoalInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $Barnyard: FUNCTION 00414e40
// $Barnyard: FUNCTION 00550ed0
AGoalCollisionUpdateGoalInterface::~AGoalCollisionUpdateGoalInterface()
{
}

// $Barnyard: FUNCTION 0054f620
void AGoalCollisionUpdateGoalInterface::OnGoalCollisionUpdate( AGoalCollisionUpdateInterface* a_pInterface )
{
	GetGoal()->UpdateHeightIfUnknown( a_pInterface );
}
