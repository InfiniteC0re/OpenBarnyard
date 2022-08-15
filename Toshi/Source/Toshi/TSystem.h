#pragma once
#include <string.h>
#include "Typedefs.h"
namespace Toshi
{
	class TSystem
	{
	public:
		static int __stdcall StringLength(char const*);
		static void* __stdcall MemCopy(void*, void const*, unsigned int);
		static char const* __stdcall StringUnicodeToChar(char*, unsigned short const*, int);
		static int __stdcall StringLength(unsigned short const*);
	};
}


