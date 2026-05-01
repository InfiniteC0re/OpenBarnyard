#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateInterface.h"

#include <Toshi/TSingleton.h>

class AInteriorNetworkManager
    : public Toshi::TSingleton<AInteriorNetworkManager>
{
public:
	AInteriorNetwork* FindNetwork( const Toshi::TPString8& a_rcName );

	void RemoveGoalInterface( AGoalCollisionUpdateInterface* a_pGoalInterface );
};
