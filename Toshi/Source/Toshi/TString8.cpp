#include "ToshiPCH.h"
#include "TString8.h"
#include "TString16.h"

#include "Toshi/TSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TString8::TString8(T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		AllocBuffer(0, TTRUE);
	}

	TString8::TString8(TString8&& src, T2Allocator* allocator) noexcept
	{
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		TString8::m_iExcessLen = src.m_iExcessLen;
		TString8::m_iStrLen = src.m_iStrLen;
		TString8::m_pBuffer = src.m_pBuffer;
		src.m_iExcessLen = 0;
		src.m_iStrLen = 0;
		src.m_pBuffer = NullString;
	}

	TString8::TString8(const TString8& src, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		Copy(src, -1);
	}

	TString8::TString8(TUINT32 size, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		AllocBuffer(size);
	}

	TString8::TString8(const TString16& src, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		Copy(src);
	}

	TString8::TString8(const TCHAR* src, T2Allocator* allocator)
	{
		Reset();
		m_pAllocator = allocator == TNULL ? GetAllocator() : allocator;
		Copy(src);
	}

	void TString8::Copy(const TString16& src, TUINT32 size)
	{
		TUINT32 srcLen = src.Length();
		TASSERT(srcLen <= 0xFFFFFF, "Too big string");

		if (srcLen < size || size == -1) { size = srcLen; }

		AllocBuffer(size, TTRUE);
		TStringManager::StringUnicodeToChar(m_pBuffer, src, size);
		m_pBuffer[size] = 0;
	}

	void TString8::Copy(const TCHAR* src, TUINT32 size)
	{
		if (src != m_pBuffer)
		{
			TUINT32 srcLen = src ? TStringManager::String8Length(src) : 0;
			TASSERT(srcLen <= 0xFFFFFF, "Too big string");

			if (srcLen < size || size == -1)
			{
				size = (TUINT32)srcLen;
			}

			AllocBuffer(size, TTRUE);
			TUtil::MemCopy(m_pBuffer, src, size);

			m_pBuffer[size] = 0;
		}
	}

	void TString8::Copy(const TWCHAR* src, TUINT32 size)
	{
		TUINT32 srcLen = TStringManager::String16Length(src);
		TASSERT(srcLen <= 0xFFFFFF, "Too big string");

		if (srcLen < size || size == -1) { size = srcLen; }

		AllocBuffer(size, TTRUE);
		TStringManager::StringUnicodeToChar(m_pBuffer, src, size);
		m_pBuffer[size] = 0;
	}

	TINT TString8::Find(TCHAR character, TUINT32 pos) const
	{
		if (!IsIndexValid(pos)) return -1;

		const TCHAR* foundAt = strchr(&m_pBuffer[pos], character);
		if (foundAt == TNULL) return -1;

		return (TINT)(foundAt - m_pBuffer);
	}

	TINT TString8::Find(const TCHAR* substr, TUINT32 pos) const
	{
		if (!IsIndexValid(pos)) return -1;

		const TCHAR* foundAt = strstr(GetString(pos), substr);
		if (foundAt == TNULL) return -1;

		return (TINT)(foundAt - m_pBuffer);
	}

	TBOOL TString8::AllocBuffer(TUINT32 a_iLength, TBOOL freeMemory)
	{
		TBOOL hasChanged = TFALSE;
		TUINT32 currentLength = Length();

		TASSERT(a_iLength >= 0, "Length can't be less than 0");
		TASSERT(a_iLength <= 0xFFFFFF, "Too big string");

		if (a_iLength != currentLength)
		{
			if (a_iLength == 0)
			{
				if (freeMemory) m_pAllocator->Free(m_pBuffer);

				m_pBuffer = NullString;
				m_iExcessLen = 0;

				hasChanged = TTRUE;
			}
			else
			{
				TINT newExcessLen = (currentLength - a_iLength) + m_iExcessLen;

				if (newExcessLen < 0 || newExcessLen > 0xFF)
				{
					if (currentLength != 0 && freeMemory)
					{
						m_pAllocator->Free(m_pBuffer);
					}

					m_pBuffer = (TCHAR*)m_pAllocator->Malloc(a_iLength + 1);
					m_iExcessLen = 0;

					hasChanged = TTRUE;
				}
				else
				{
					m_iExcessLen = newExcessLen;
					hasChanged = TFALSE;
				}
			}

			m_iStrLen = a_iLength;
		}

		if (freeMemory) m_pBuffer[0] = '\0';
		return hasChanged;
	}

	TString8 TString8::Format(const TCHAR* a_pcFormat, ...)
	{
		TCHAR buffer[0x400];
		TString8 buffer2;
		va_list args;

		va_start(args, a_pcFormat);

		// TASSERT is only in T2String8
		TINT iResult = _vsnprintf(buffer, sizeof(buffer), a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
		buffer2.Copy(buffer, -1);
		
		return buffer2;
	}

	TString8& TString8::VFormat(const TCHAR* a_pcFormat, va_list a_vargs)
	{
		TCHAR buffer[0x400];

		// TASSERT is only in T2String8
		TINT iResult = _vsnprintf(buffer, sizeof(buffer), a_pcFormat, a_vargs);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem");
		Copy(buffer, -1);

		return *this;
	}

	void TString8::ForceSetData(TCHAR* a_cString, TINT a_iLength)
	{
		m_pBuffer = a_cString;

		if (a_iLength < 0)
		{
			m_iStrLen = TStringManager::String8Length(a_cString);
		}
		else
		{
			m_iStrLen = a_iLength;
		}

		m_iExcessLen = 0;
	}

	TINT TString8::FindReverse(TCHAR a_findChar, TINT pos) const
	{
		if (pos == -1)
		{
			pos = m_iStrLen;
		}
		else
		{
			if (!IsIndexValid(pos)) return -1;
		}


		for (; pos > -1; pos--)
		{
			if (a_findChar == m_pBuffer[pos])
			{
				return pos;
			}
		}

		return -1;
	}

	void TString8::Truncate(TUINT32 length)
	{
		if (Length() < length)
		{
			length = Length();
		}

		TCHAR* oldBuffer = m_pBuffer;

		TBOOL allocated = AllocBuffer(length, TFALSE);
		if (allocated)
		{
			TStringManager::String8Copy(m_pBuffer, oldBuffer, length);
		}

		m_pBuffer[length] = 0;

		if (allocated && Length() != 0)
		{
			m_pAllocator->Free(oldBuffer);
		}
	}

	void TString8::FreeBuffer()
	{
		if (Length() != 0) m_pAllocator->Free(m_pBuffer);
		Reset();
	}

	const TCHAR* TString8::GetString(TUINT32 a_iIndex) const
	{
		TASSERT(a_iIndex >= 0 && a_iIndex <= (TINT)m_iStrLen);
		if (IsIndexValid(a_iIndex)) { return &m_pBuffer[a_iIndex]; }
		return TNULL;
	}

	TString8& TString8::Concat(const TCHAR* str, TUINT32 size)
	{
		TUINT32 len = (TUINT32)TStringManager::String8Length(str);
		
		if ((len < size) || (size == -1))
		{
			size = len;
		}

		TUINT32 oldLength = m_iStrLen;
		TCHAR* oldString = m_pBuffer;

		TBOOL allocated = AllocBuffer(m_iStrLen + size, TFALSE);

		if (allocated)
		{
			// since it has made a new buffer
			// it need to copy the old string
			// to the new buffer

			TStringManager::String8Copy(m_pBuffer, oldString, -1);
		}

		TStringManager::String8Copy(m_pBuffer + oldLength, str, size);
		m_pBuffer[m_iStrLen] = 0;

		if (allocated && oldLength != 0)
		{
			m_pAllocator->Free(oldString);
		}

		return *this;
	}

	TINT TString8::Compare(const TCHAR* a_pcString, TINT param_2) const
	{
		TASSERT(a_pcString != TNULL, "TCString::CompareNoCase - Passed string cannot be TNULL");
		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		TASSERT(GetString() != TNULL, "TCString::CompareNoCase - String cannot be TNULL");

		if (param_2 != -1)
		{
			return strncmp(GetString(), a_pcString, param_2);
		}

		const TCHAR* str = GetString();
		TCHAR bVar1 = 0;
		TCHAR bVar4 = 0;
		while (TTRUE)
		{
			bVar1 = *str;
			bVar4 = bVar1 < *a_pcString;

			if (bVar1 != *a_pcString) break;
			if (bVar1 == 0) return 0;
			
			bVar1 = str[1];
			bVar4 = bVar1 < a_pcString[1];

			if (bVar1 != a_pcString[1]) break;
			if (bVar1 == 0) return 0;

			str += 2;
			a_pcString += 2;
		}
		return bVar4 | 1;
	}

	TINT TString8::CompareNoCase(const TCHAR* a_pcString, TINT param_2) const
	{
		TASSERT(a_pcString != TNULL, "TCString::CompareNoCase - Passed string cannot be TNULL");
		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		TASSERT(GetString() != TNULL, "TCString::CompareNoCase - String cannot be TNULL");

		if (param_2 == -1)
		{
			return _stricmp(GetString(), a_pcString);
		}

		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		
		return _strnicmp(GetString(), a_pcString, param_2);
	}

	TString8 TString8::Mid(TUINT32 param_1, TUINT32 param_2) const
	{
		if (param_2 < 0)
		{
			param_2 = 0;
		}
		else if (Length() <= param_2)
		{
			return TString8();
		}

		// Rewrite not correct
		TString8 str = TString8(Length() - param_2);
		TUtil::MemCopy(str.m_pBuffer, GetString(param_2), Length() - param_2);
		m_pBuffer[m_iStrLen - param_2] = 0;

		return str;
	}

	TString8& TString8::Concat(const TString16& str, TUINT32 size)
	{
		TUINT32 len = str.Length();

		if ((len < size) || (size == -1))
		{
			size = len;
		}

		TUINT32 oldLength = m_iStrLen;
		TCHAR* oldString = m_pBuffer;
		
		TBOOL allocated = AllocBuffer(m_iStrLen + size, TFALSE);
		
		if (allocated)
		{
			// since it has made a new buffer
			// it need to copy the old string
			// to the new buffer

			TStringManager::String8Copy(m_pBuffer, oldString, -1);
		}

		TStringManager::StringUnicodeToChar(m_pBuffer + oldLength, str.GetString(), size);
		m_pBuffer[m_iStrLen] = 0;
		
		if (allocated && m_iStrLen != 0)
		{
			m_pAllocator->Free(oldString);
		}

		return *this;
	}

	TBOOL TString8::IsAllLowerCase() const
	{
		if (m_iStrLen != 0)
		{
			TCHAR* iter = m_pBuffer;
			TCHAR* endStr = m_pBuffer + m_iStrLen;

			do
			{
				TCHAR currentC = *iter++;
				if (currentC > '@' && currentC < '[') return TFALSE;

			} while (iter < endStr);
		}

		return TTRUE;
	}

	TBOOL TString8::IsAllUpperCase() const
	{
		if (m_iStrLen != 0)
		{
			TCHAR* iter = m_pBuffer;
			TCHAR* endStr = m_pBuffer + m_iStrLen;

			do
			{
				TCHAR currentC = *iter++;
				if (currentC > '`' && currentC < '{') return TFALSE;

			} while (iter < endStr);
		}

		return TTRUE;
	}
}