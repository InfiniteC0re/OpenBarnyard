#pragma once
#include "Toshi2/T2String8.h"
#include "Toshi2/T2String16.h"

namespace Toshi {

	class TStringManager
	{
	public:
		static char* GetTempString8() { return T2String8::ms_aScratchMem; }
		static int String8Compare(const char* str1, const char* str2, size_t size = -1) { return T2String8::Compare(str1, str2, size); }
		static int String8CompareNoCase(const char* str1, const char* str2, size_t size = -1) { return T2String8::CompareNoCase(str1, str2, size); }
		static char* String8Copy(char* dst, const char* src, size_t size = -1) { return T2String8::Copy(dst, src, size); }
		static char* String8CopySafe(char* dst, const char* src, size_t size = -1) { return T2String8::CopySafe(dst, src, size); }
		static const char* String8FindChar(const char* str, char character) { return T2String8::FindChar(str, character); }
		static const char* String8FindString(const char* str, const char* substr) { return T2String8::FindString(str, substr); }
		static size_t String8Length(const char* str) { return T2String8::Length(str); }
		static void String8ToLowerCase(char* str) { T2String8::ToLowerCase(str); }
		static TFLOAT String8ToFloat(const char* str) { return T2String8::StringToFloat(str); }

		static void String8Format(char* str, int size, const char* format, ...) 
		{
			va_list args;

			va_start(args, format);

			T2String8::FormatV(str, size, format, args);

			va_end(args);
		}

		static wchar_t* GetTempString16() { return T2String16::ms_aScratchMem; }
		static int String16Compare(const wchar_t* str1, const wchar_t* str2, size_t size = -1) { return T2String16::Compare(str1, str2, size); }
		static int String16CompareNoCase(const wchar_t* str1, const wchar_t* str2, size_t size = -1) { return T2String16::CompareNoCase(str1, str2, size); }
		static wchar_t* String16Copy(wchar_t* dst, const wchar_t* src, size_t size = -1) { return T2String16::Copy(dst, src, size); }
		static wchar_t* String16CopySafe(wchar_t* dst, const wchar_t* src, size_t size = -1) { return T2String16::CopySafe(dst, src, size); }
		static const wchar_t* String16FindChar(const wchar_t* str, wchar_t character) { return T2String16::FindChar(str, character); }
		static const wchar_t* String16FindString(const wchar_t* str, wchar_t* substr) { return T2String16::FindString(str, substr); }
		static size_t String16Length(const wchar_t* str) { return T2String16::Length(str); }
		
		static void String16Format(wchar_t* str, int size, const wchar_t* format, ...)
		{
			va_list args;

			va_start(args, format);

			T2String16::FormatV(str, size, format, args);

			va_end(args);
		}

		static wchar_t* StringCharToUnicode(wchar_t* a_UnicodeString, const char* a_CharString, uint32_t a_iLength);
		static char* StringUnicodeToChar(char* a_CharString, const wchar_t* const& a_UnicodeString, uint32_t a_iLength);
	};

}
