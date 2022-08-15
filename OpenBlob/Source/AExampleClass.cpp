#include "AExampleClass.h"
#include <iostream>

TOSHI_CLASS_INITIALIZE(AExampleClass, nullptr, 1, 0)

AExampleClass::AExampleClass()
{
	TOSHI_INFO("Called AExampleClass");
}

AExampleClass::~AExampleClass()
{
	TOSHI_INFO("Called ~AExampleClass");
}