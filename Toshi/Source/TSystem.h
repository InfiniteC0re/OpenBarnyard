#pragma once
#include <string.h>
namespace Toshi
{
	class TSystem
	{
	public:
		static int __stdcall StringLength(char const*);
		static void* __stdcall MemCopy(void*, void const*, unsigned int);
	};
}


