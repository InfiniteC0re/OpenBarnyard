#pragma once
#include "ARootTask.h"

#include <Toshi/File/TFile.h>
#include <Toshi/Core/TSystem.h>
#include <Toshi/Core/TKernelInterface.h>
#include <Toshi/Core/TScheduler.h>
#include <Toshi/Math/TRandom.h>
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/TGenericDList.h>
#include <MemoryCard/EnSaveData.h>
#include <Toshi/Xui/TXUIScene.h>

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
