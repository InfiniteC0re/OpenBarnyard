#pragma once
#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// An empty class registered in the retail game. Nothing creates or uses it.
//-----------------------------------------------------------------------------
class AGameStateReturnValue : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AGameStateReturnValue, Toshi::TObject );

public:
	AGameStateReturnValue() = default;
	~AGameStateReturnValue();
};
