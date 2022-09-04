#pragma once
#include "ARootTask.h"

//-----------------------------------------------------------
// this is an example of registering and using TClass objects
//-----------------------------------------------------------

class AExampleClass : public Toshi::TObject
{
	TOSHI_CLASS_DEFINE(AExampleClass)

public:
	AExampleClass();
	~AExampleClass();

private:
	ARootTask* m_RootTask;
};

