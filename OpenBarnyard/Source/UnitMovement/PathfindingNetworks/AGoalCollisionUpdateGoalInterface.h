#pragma once

#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateInterface.h"

class AInteriorGoal;

class AGoalCollisionUpdateGoalInterface : public AGoalCollisionUpdateInterface
{
public:
	AGoalCollisionUpdateGoalInterface() = default;
	~AGoalCollisionUpdateGoalInterface() override;
};
