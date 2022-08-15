#include "pch.h"
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

char const* __stdcall Toshi::TSystem::StringUnicodeToChar(char* a_CharString, unsigned short const* a_UnicodeString, int a_iLength)
{
	TASSERT((a_UnicodeString != TNULL) && (a_CharString != NULL));
	
	int iVar2 = StringLength(a_UnicodeString);
	if ((iVar2 < a_iLength) || (a_iLength == -1))
	{
		a_iLength = iVar2;
	}
	iVar2 = 0;
	if (a_iLength > 0)
	{
		do
		{
			a_CharString[iVar2] = a_UnicodeString[iVar2];
		} while (iVar2 < a_iLength);
	}
	a_CharString[iVar2] = '\0';
	return a_CharString;
}

int __stdcall Toshi::TSystem::StringLength(unsigned short const* a_UnicodeString)
{
	// In de Blob own StringLength func
	return wcslen((wchar_t*)a_UnicodeString);
}
