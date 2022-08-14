#include "TSystem.h"

int __stdcall Toshi::TSystem::StringLength(char const*)
{
    return 0;
}

void* __stdcall Toshi::TSystem::MemCopy(void* dst, void const* src, unsigned int size)
{
	//JPOG implemented their own memcpy
	return memcpy(dst, src, size);
}