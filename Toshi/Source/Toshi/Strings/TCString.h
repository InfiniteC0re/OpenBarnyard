#pragma once
#include "TString.h"
#include "TWString.h"
#include "Toshi/Typedefs.h"

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

		inline void Copy(const TCString& src, uint32_t size = -1) { Copy(src.m_pBuffer, size); }
		void Copy(const TWString& src, uint32_t size = -1);
		void Copy(const char* src, uint32_t size = -1);

		void FreeBuffer();

		// Returns true if allocated memory
		bool AllocBuffer(int size, bool freeMemory = true);

		// Returns position of specified character
		uint32_t Find(char substr, int pos = -1) const;
		int Find(const char*, int) const;

		// Returns string starting from specified index
		const char* GetString(uint32_t index = 0) const;

		inline TCString& Concat(const TCString& str, uint32_t size = -1) { return Concat(str.m_pBuffer, size); };
		TCString& Concat(const TWString& src, uint32_t size = -1);
		TCString& Concat(const char* src, uint32_t size = -1);

		int Compare(const char*, int) const;
		int CompareNoCase(const char*, int) const;

		inline TCString& MakeUpper() { if (m_pBuffer != TNULL) { strupr(m_pBuffer); } return *this; }
		inline TCString& MakeLower() { if (m_pBuffer != TNULL) { strlwr(m_pBuffer); } return *this; }

		inline uint32_t Length() const { return m_iStrLen; }
		inline uint8_t ExcessLength() const { return m_iExcessLen; }

		inline bool IsIndexValid(uint32_t index) const { return index >= 0 && index <= Length(); }
		inline bool IsEmpty() { return m_iStrLen == 0; }
		inline bool IsUnicode() { return false; } // Who would have known?


	public:
		TCString operator+(char const*) const;
		TCString* operator+= (char const*);
		inline operator const char* () const { return m_pBuffer; }
		inline char& operator[](int index) { return m_pBuffer[index]; }
		inline const char& operator[](int param_1) const { return GetString()[param_1]; }
		inline bool operator!() { return m_iStrLen == 0; }
		inline bool operator!=(char* param_1) { return Compare(param_1, -1) != 0; }
		inline bool operator!=(const TCString& param_1) { return Compare(param_1.GetString(), -1) != 0; }
		inline bool operator<(const char* param_1) const { return Compare(param_1, -1) > -1; };
		inline bool operator<(const TCString& param_1) const { return Compare(param_1.GetString(), -1) > -1; };
		inline bool operator<=(const char* param_1) const { return Compare(param_1, -1) > 0; };
		inline bool operator<=(const TCString& param_1) const { return Compare(param_1.GetString(), -1) > 0; };
		//inline TCString& operator=(const unsigned short* param_1) { Copy(param_1, -1); return *this; };
		inline TCString& operator=(const char* param_1) { Copy(param_1, -1); return *this; };



	private:
		void Reset();

	private:
		char* m_pBuffer = NullString;
		uint8_t m_iExcessLen = 0;
		uint32_t m_iStrLen : 24 = 0;
	};
}


