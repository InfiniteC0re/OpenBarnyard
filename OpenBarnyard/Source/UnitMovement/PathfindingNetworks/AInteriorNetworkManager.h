#pragma once

#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"

class AInteriorNetworkManager
{
public:
	AInteriorNetwork* FindNetwork( const Toshi::TPString8& a_rcName );
};
