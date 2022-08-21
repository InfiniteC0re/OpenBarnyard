#pragma once
#include "TString.h"
#include "TWString.h"

namespace Toshi
{
	class TCString
	{
	public:
		TCString();
		TCString(const TCString& src);
		TCString(const TWString& src);
		TCString(const char* const& src);
		TCString(uint32_t size);
		~TCString();

		inline void Copy(const TCString& src, uint32_t size) { Copy(src.m_pBuffer, size); }
		void Copy(const TWString& src, uint32_t size);
		void Copy(const char* src, uint32_t size);

		void FreeBuffer();

		// Returns true if updated memory
		bool AllocBuffer(int size, bool freeMemory);

		// Returns position of specified substring
		uint32_t Find(char substr, int pos) const;

		// Returns string starting from specified index
		const char* GetString(uint32_t index = 0) const;

		inline uint32_t Length() const { return m_iStrLen; }
		inline uint8_t ExcessLength() const { return m_iExcessLen; }
		inline bool IsIndexValid(uint32_t index) const { return index >= 0 && index <= Length(); }

	public:
		TCString* operator+= (char const*);
		inline operator const char* () const { return m_pBuffer; }
		char& operator[](int);

	private:
		void Reset();

	private:
		char* m_pBuffer = NullString;
		uint8_t m_iExcessLen : 8 = 0;
		uint32_t m_iStrLen : 24 = 0;
	};
}


