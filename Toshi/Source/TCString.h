#pragma once
#include "Typedefs.h"
#include <string.h>
#include "TSystem.h"
#include "TMemory.h"
namespace Toshi
{
	class TCString
	{
		char** m_pBuffer;
		unsigned int unk2;
	private:
		static char* m_aNull;
	public:
		__thiscall TCString();

		void __thiscall Copy(class TCString const*, int);
		void __thiscall Copy(char const*, int);
		bool __thiscall IsIndexValid(int) const;
		int __thiscall Find(char, int) const;
		bool __thiscall AllocBuffer(int, bool);
	};
}


