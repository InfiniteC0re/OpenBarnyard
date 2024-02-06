#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi {

	template <TUINT Size>
	class T2FixedString
	{
	public:
		TSTATICASSERT(Size > 0);
		TSTATICASSERT(Size <= 4096);

	public:
		T2FixedString()
		{
			m_szBuffer[0] = '\0';
		}

		T2FixedString(const char* a_szString)
		{
			T2String8::CopySafe(m_szBuffer, a_szString, Size);
		}
		
		void Format(const char* a_szFormat, ...)
		{
			va_list args;

			va_start(args, a_szFormat);
			T2String8::FormatV(m_szBuffer, Size, a_szFormat, args);
			va_end(args);

			m_szBuffer[Size - 1] = '\0';
		}

		void Append(const char* a_szString)
		{
			T2String8::Concat(m_szBuffer, a_szString);
		}

		void Clear() { m_szBuffer[0] = '\0'; }

		TUINT Length() { return T2String8::Length(m_szBuffer); }

		const char* Get() const { return m_szBuffer; }

	private:
		char m_szBuffer[Size];
	};

	using T2FixedString32 = T2FixedString<32>;
	using T2FixedString64 = T2FixedString<64>;
	using T2FixedString128 = T2FixedString<128>;
	using T2FixedString256 = T2FixedString<256>;
	using T2FixedString512 = T2FixedString<512>;

}