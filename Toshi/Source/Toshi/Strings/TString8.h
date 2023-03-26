#pragma once
#include "TString.h"
#include "TString16.h"
#include "Toshi/Typedefs.h"
#include "Toshi2/T2Allocator.h"

namespace Toshi
{
	class TString8
	{
	public:
		TString8(T2Allocator* allocator = TNULL);
		TString8(TString8&& src, T2Allocator* allocator = TNULL) noexcept;
		TString8(const TString8& src, T2Allocator* allocator = TNULL);
		TString8(const TString16& src, T2Allocator* allocator = TNULL);
		TString8(const char* const& src, T2Allocator* allocator = TNULL);
		TString8(uint32_t size, T2Allocator* allocator = TNULL);
		~TString8() { FreeBuffer(); }

		void Copy(const TString8& src, uint32_t size = -1) { Copy(src.m_pBuffer, size); }
		void Copy(const TString16& src, uint32_t size = -1);
		void Copy(const char* src, uint32_t size = -1);

		void FreeBuffer();

		// Returns true if allocated memory
		bool AllocBuffer(uint32_t size, bool freeMemory = true);

		TString8 Format(const char* a_pcFormat, ...);
		TString8& VFormat(const char* a_pcFormat, char* a_pcArgs);

		void UndoForceSetData() { Reset(); }
		void ForceSetData(char* a_cString, int a_ilength);

		int FindReverse(char a_findChar, int pos) const;

		void Truncate(uint32_t length);

		// Returns position of specified character
		int32_t Find(char character, uint32_t pos = 0) const;

		// Returns position of specified substring
		int32_t Find(const char* substr, uint32_t pos = 0) const;

		// Returns string starting from specified index
		const char* GetString(uint32_t index = 0) const;

		TString8& Concat(const TString8& str, uint32_t size = -1) { return Concat(str.m_pBuffer, size); };
		TString8& Concat(const TString16& src, uint32_t size = -1);
		TString8& Concat(const char* src, uint32_t size = -1);

		int Compare(const char*, int) const;
		int CompareNoCase(const char*, int) const;

		TString8 Mid(uint32_t param_1, uint32_t param_2) const;
		TString8 Right(int param_1) const { return Mid(param_1, Length() - param_1); }

		TString8& MakeUpper() { _strupr_s(m_pBuffer, m_iStrLen); return *this; }
		TString8& MakeLower() { _strlwr_s(m_pBuffer, m_iStrLen); return *this; }

		uint32_t Length() const { return m_iStrLen; }
		uint8_t ExcessLength() const { return m_iExcessLen; }

		bool IsAllLowerCase() const;
		bool IsAllUpperCase() const;
		bool IsIndexValid(uint32_t index) const { return index >= 0 && index <= Length(); }
		bool IsEmpty() const { return m_iStrLen == 0; }
		bool IsUnicode() const { return false; } // Who would have known?

	public:
		TString8 operator+(char const* cstr) const { TString8 str = TString8(*this); return std::move(str.Concat(cstr)); }
		TString8* operator+= (char const* cstr) { Concat(cstr, -1); return this; }
		TString8* operator+= (TString8& str) { Concat(str, -1); return this; }
		char& operator[](int index) { return m_pBuffer[index]; }
		const char& operator[](int index) const { return *GetString(index); }
		operator const char* () const { return m_pBuffer; }
		bool operator!() { return m_iStrLen == 0; }
		bool operator==(const char* cstr) const { return Compare(cstr, -1) == 0; }
		bool operator==(const TString8& str) const { return Compare(str.m_pBuffer, -1) == 0; }
		bool operator!=(const char* cstr) const { return Compare(cstr, -1) != 0; }
		bool operator!=(const TString8& str) const { return Compare(str.m_pBuffer, -1) != 0; }
		bool operator<(const char* cstr) const { return Compare(cstr, -1) > -1; };
		bool operator<(const TString8& str) const { return Compare(str.m_pBuffer, -1) > -1; };
		bool operator<=(const char* cstr) const { return Compare(cstr, -1) > 0; };
		bool operator<=(const TString8& str) const { return Compare(str.m_pBuffer, -1) > 0; };
		//TString8& operator=(const wchar_t* wcstr) { Copy(wcstr, -1); return *this; };
		TString8& operator=(const char* cstr) { Copy(cstr, -1); return *this; };
		TString8& operator=(const TString8& str) { Copy(str, -1); return *this; };

	private:

		typedef T2Allocator* (*func_DefaultAllocatorCB)();

		void Reset()
		{
			m_pBuffer = NullString;
			m_iStrLen = 0;
			m_iExcessLen = 0;
		}

		T2Allocator* GetAllocator()
		{
			return sm_pDefaultAllocatorCB();
		}

		static T2Allocator* GetDefaultAllocatorCB()
		{
			return &T2Allocator::s_GlobalAllocator;
		}

	private:

		static inline func_DefaultAllocatorCB sm_pDefaultAllocatorCB = &GetDefaultAllocatorCB;

		char* m_pBuffer = NullString; // 0x0
		uint8_t m_iExcessLen = 0;     // 0x4
		uint32_t m_iStrLen : 24 = 0;  // 0x5
		T2Allocator* m_pAllocator;    // 0x8
	};
}


