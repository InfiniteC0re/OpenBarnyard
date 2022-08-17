#pragma once
#include "TWString.h"

namespace Toshi
{
	class TCString
	{
	public:
		__thiscall TCString();
		TCString(int param_1);
		TCString(char* param_1);

		Toshi::TCString* operator+= (char const*);
		operator const char* ();

		void __thiscall Copy(class Toshi::TWString *, int);
		void __thiscall Copy(class TCString const*, int);
		void __thiscall Copy(char const*, int);
		bool __thiscall IsIndexValid(int) const;
		int __thiscall Find(char, int) const;
		bool __thiscall AllocBuffer(int, bool);
		int Length() const;
		const char* GetString(int param_1) const;
	private:
		void Reset();

		char** m_pBuffer;
		unsigned int length = 0;
	};
}


