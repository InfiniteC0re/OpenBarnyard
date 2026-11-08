#pragma once
#include "Platform/WASM/TTypes_WASM.h"

#define _A_SUBDIR 0x10

struct _finddata_t
{
	unsigned attrib;
	long     size;
	char     name[ 260 ];
};

intptr_t _findfirst( const char* a_szPath, _finddata_t* a_pData );
int      _findclose( intptr_t a_hFind );
