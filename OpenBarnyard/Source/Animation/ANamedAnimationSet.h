#pragma once
#include <Toshi/TRefCounted.h>

class ANamedAnimationSet :
    public Toshi::TRefCounted
{
public:
	void Delete() {}
};

using ANamedAnimationSetRef = Toshi::TRef<ANamedAnimationSet>;
