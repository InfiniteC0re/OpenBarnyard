#pragma once

#include "UnitMovement/PathfindingNetworks/AGoalCollisionUpdateInterface.h"

class AInteriorGoal;

class AGoalCollisionUpdateGoalInterface : public AGoalCollisionUpdateInterface
{
public:
	// $Barnyard: FUNCTION 00414fd0
	AGoalCollisionUpdateGoalInterface() {}
	~AGoalCollisionUpdateGoalInterface() override;

	virtual void           OnGoalCollisionUpdate( AGoalCollisionUpdateInterface* a_pInterface ) override; // +0x04
	virtual AInteriorGoal* GetGoal() override { return m_pGoal; }                                         // +0x08, folded getter 004ffb80
};
