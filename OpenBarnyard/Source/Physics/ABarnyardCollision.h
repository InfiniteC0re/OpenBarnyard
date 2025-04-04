#pragma once
#include "APhysics.h"

#include <Toshi/TSingleton.h>

class ABarnyardCollision
    : public APhysics::ACollisionInterface<APhysics::ASweptSphere>
    , public APhysics::ACollisionInterface<APhysics::ASphere>
    , public APhysics::ACollisionInterface<APhysics::ARay>
    , public Toshi::TSingleton<ABarnyardCollision>
{
public:
	
};
