#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/T2SList.h>

class AAnimatableObject
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimatableObject>::Node
    , public Toshi::T2SList<AAnimatableObject>::Node
{
public:
	TDECLARE_CLASS( AAnimatableObject, Toshi::TObject );

public:
	AAnimatableObject();
	~AAnimatableObject();


};
