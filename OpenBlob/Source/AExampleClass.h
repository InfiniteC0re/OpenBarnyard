#pragma once
#include "ARootTask.h"

//-----------------------------------------------------------
// this is an example of registering and using TClass objects
//-----------------------------------------------------------

class AExampleClass :
	public Toshi::TGenericClassDerived<AExampleClass, Toshi::TObject, "AExampleClass", TMAKEVERSION(1, 0), false>
{
public:
	AExampleClass();
	~AExampleClass();

private:
	ARootTask* m_RootTask;
};

