#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// TOSHI_TMEMORY_SIZE allows you to change size of the global memory block
// The value is not used when TMemory initialized with native methods
#define TOSHI_TMEMORY_SIZE 0

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"

#include "TRBF/TRBF.h"

int TMain(int argc, char** argv)
{
	TRBF trbf;

	SECT* pSect = trbf.GetSECT();
	SECT::Stack* pStack = pSect->CreateSection();

	struct MyStruct
	{
		int a = 6;
		int b = 6;
		MyStruct* pStruct = TNULL;
	};

	auto ptr1 = pStack->Alloc<float>();
	auto ptr2 = pStack->Alloc<MyStruct>();
	
	ptr2->a = 9;
	ptr2->b = 10;
	pStack->Alloc<MyStruct>(&ptr2->pStruct);
	
	ptr2->pStruct->a = 11;
	ptr2->pStruct->b = 12;
	ptr2->pStruct->pStruct = TNULL;

	auto ptr3 = pStack->Alloc<float>();
	*ptr3 = 1.69f;
	ptr2->pStruct->b = 19;

	trbf.WriteToFile("D:\\test.trb");

	return 0;
}