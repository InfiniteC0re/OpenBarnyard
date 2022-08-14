#include "TMemory.h"

void __stdcall tfree(void * toFree)
{

}

void* __stdcall tmalloc(int, char*, int)
{
	return nullptr;
}

void* __stdcall tmalloc(int)
{
	return nullptr;
}


