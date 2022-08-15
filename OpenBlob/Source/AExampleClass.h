#pragma once
#include "Toshi/TObject.h"

//-----------------------------------------------------------
// this is an example of registering and using TClass objects
//-----------------------------------------------------------

class AExampleClass : public Toshi::TObject
{
	TOSHI_CLASS_REGISTER(AExampleClass)

public:
	AExampleClass();
	~AExampleClass();
};
