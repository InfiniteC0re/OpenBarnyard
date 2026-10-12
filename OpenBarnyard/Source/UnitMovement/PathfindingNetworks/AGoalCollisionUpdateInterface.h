#pragma once
#include "AInteriorGoal.h"

#include <Toshi/T2SList.h>

class AGoalCollisionUpdateInterface
    : public Toshi::T2SList<AGoalCollisionUpdateInterface>::Node
{
public:
	friend class AInteriorNetwork;

public:
	virtual ~AGoalCollisionUpdateInterface();                                                      // +0x00
	virtual void           OnGoalCollisionUpdate( AGoalCollisionUpdateInterface* a_pInterface ) {} // +0x04, DOES_NOTHING (vtable 0x00717df4)
	virtual AInteriorGoal* GetGoal() { return TNULL; }                                             // +0x08, RETURNS_ZERO

protected:
	AInteriorGoal* m_pGoal;
};
