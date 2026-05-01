#pragma once
#include "AInteriorGoal.h"

#include <Toshi/T2SList.h>

class AGoalCollisionUpdateInterface
    : public Toshi::T2SList<AGoalCollisionUpdateInterface>::Node
{
public:
	friend class AInteriorNetwork;

public:
	virtual ~AGoalCollisionUpdateInterface();

	// TODO: more virtual tables

protected:
	AInteriorGoal* m_pGoal;
};
