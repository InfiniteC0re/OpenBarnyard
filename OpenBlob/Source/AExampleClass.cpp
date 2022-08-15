#include "AExampleClass.h"
#include <iostream>

TOSHI_CLASS_INITIALIZE(AExampleClass, nullptr, 1, 0)

AExampleClass::AExampleClass()
{
	std::cout << "Called AExampleClass" << std::endl;
}

AExampleClass::~AExampleClass()
{
	std::cout << "Called ~AExampleClass" << std::endl;
}