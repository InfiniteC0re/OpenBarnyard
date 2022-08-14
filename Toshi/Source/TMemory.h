#pragma once
namespace Toshi
{
	class TMemory
	{
	};
}

void __stdcall tfree(void *);
void* __stdcall tmalloc(int, char*, int);
void* __stdcall tmalloc(int);

