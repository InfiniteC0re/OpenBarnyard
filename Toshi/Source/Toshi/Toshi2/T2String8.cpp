#include "ToshiPCH.h"
#include "T2String8.h"

int Toshi2::T2String8::CompareStrings(const char* a_String1, const char* a_String2, int a_MaxCount)
{
	TASSERT(a_String1 != TNULL && a_String2 != TNULL, "Strings can't be TNULL");
	return a_MaxCount != -1 ? _strnicmp(a_String1, a_String2, a_MaxCount) : _stricmp(a_String1, a_String2);
}
