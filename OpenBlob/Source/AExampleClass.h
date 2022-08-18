#pragma once
#include "Toshi/Core/Core.h"
#include "Toshi/Core/TObject.h"

#include "Toshi/Utils/TLog.h"

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
