#pragma once
#include "Toshi/Core/TObject.h"

//-----------------------------------------------------------
// this is an example of registering and using TClass objects
//-----------------------------------------------------------

class AExampleClass : public Toshi::TObject
{
	TOSHI_CLASS_DEFINE(AExampleClass)

public:
	AExampleClass();
	~AExampleClass();
};


class AExampleClass2 : public Toshi::TObject
{
	TOSHI_CLASS_DEFINE(AExampleClass2)

public:
	AExampleClass2();
	~AExampleClass2();
};
