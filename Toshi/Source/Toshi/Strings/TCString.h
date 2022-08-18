#pragma once
#include "TWString.h"
#include "Toshi/Core/TMemory.h"
#include "TString.h"

namespace Toshi
{
	class TCString
	{
	public:
		TCString();
		TCString(uint32_t length);
		TCString(const char* const& src);
		~TCString();

		inline void Copy(const TCString& src, uint32_t size) { Copy(src.m_pBuffer, size); }
		void Copy(const TWString& src, uint32_t size);
		void Copy(const char* src, uint32_t size);

		void FreeBuffer();
		bool AllocBuffer(int, bool);

		int Find(char substr, int pos) const;

		inline uint32_t Length() const { return m_iStrLen; }
		inline bool IsIndexValid(uint32_t index) const { return index <= Length() && index >= 0; }

		const char* GetString(uint32_t index) const;

	public:
		TCString* operator+= (char const*);
		inline operator const char* () const { return m_pBuffer; }

	private:
		void Reset();

	private:
		char* m_pBuffer = NullString;
		int m_iExcessLen : 8;
		int m_iStrLen : 24;
	};
}


