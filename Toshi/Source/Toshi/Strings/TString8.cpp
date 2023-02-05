#include "ToshiPCH.h"
#include "TString8.h"

#include "Toshi/Core/TSystem.h"

namespace Toshi
{
	TString8::TString8()
	{
		Reset();
		AllocBuffer(0, true);
		// this+7 = 0
		/*  if (param_1 == 0) {
		if ((_DAT_009b1704 == 0) && (sm_pDefaultAllocatorCB == (code *)0x0)) {
		  puVar1 = (undefined4 *)
				   Assert("TNULL!=sm_pDefaultAllocatorCB","..\\..\\Source\\Toshi\\TString8.cpp",100,
						  "TASSERT");
		  return puVar1;
		}
		param_1 = (*sm_pDefaultAllocatorCB)();
	  }
	  *(int *)((int)this + 8) = param_1;
	  uVar2 = (*(int *)((int)this + 4) << 8) >> 8;
	  if ((_DAT_009b1700 == 0) && (0x7fffffff < uVar2)) {
		puVar1 = (undefined4 *)
				 Assert("a_iX <= TINT32_MAX",
						"E:\\src\\Development\\ToshiBranch\\Source\\Math/TMathInline.h",0x67,"TASSERT");
		return puVar1;
	  }
	  if (uVar2 != 0) {
		(**(code**)(**(int**)((int)this + 8) + 8))(*this);
		*(undefined*)((int)this + 7) = 0;
		*(uint*)((int)this + 4) = *(uint*)((int)this + 4) & 0xff000000;
		*(undefined1**)this = &m_aNull;
	}
	**this = 0;
	return (undefined4*)this;
		*/
	}

	TString8::TString8(TString8&& src) noexcept
	{
		TString8::m_iExcessLen = src.m_iExcessLen;
		TString8::m_iStrLen = src.m_iStrLen;
		TString8::m_pBuffer = src.m_pBuffer;
		src.m_iExcessLen = 0;
		src.m_iStrLen = 0;
		src.m_pBuffer = NullString;
	}

	TString8::TString8(const TString8& src)
	{
		Reset();
		Copy(src, -1);
	}

	TString8::TString8(uint32_t size)
	{
		Reset();
		AllocBuffer(size);
	}

	TString8::TString8(const TString16& src)
	{
		Reset();
		Copy(src);
	}

	TString8::TString8(const char* const& src)
	{
		Reset();
		Copy(src);
	}

	void TString8::Copy(const TString16& src, uint32_t size)
	{
		uint32_t srcLen = src.Length();
		TASSERT(srcLen <= 0xFFFFFF, "Too big string");

		if (srcLen < size || size == -1) { size = srcLen; }

		AllocBuffer(size, true);
		TStringManager::StringUnicodeToChar(m_pBuffer, src, size);
		m_pBuffer[size] = 0;
	}

	void TString8::Copy(const char* src, uint32_t size)
	{
		if (src != m_pBuffer)
		{
			size_t srcLen = src ? TStringManager::String8Length(src) : 0;
			TASSERT(srcLen <= 0xFFFFFF, "Too big string");

			if (srcLen < size || size == -1)
			{
				size = (uint32_t)srcLen;
			}

			AllocBuffer(size, true);
			TUtil::MemCopy(m_pBuffer, src, size);

			m_pBuffer[size] = 0;
		}
	}

	int32_t TString8::Find(char character, uint32_t pos) const
	{
		if (!IsIndexValid(pos)) return -1;

		const char* foundAt = strchr(&m_pBuffer[pos], character);
		if (foundAt == nullptr) return -1;

		return (int32_t)(foundAt - m_pBuffer);
	}

	int32_t TString8::Find(const char* substr, uint32_t pos) const
	{
		if (!IsIndexValid(pos)) return -1;

		const char* foundAt = strstr(GetString(pos), substr);
		if (foundAt == nullptr) return -1;

		return (int32_t)(foundAt - m_pBuffer);
	}

	bool TString8::AllocBuffer(uint32_t a_iLength, bool freeMemory)
	{
		bool hasChanged = false;
		uint32_t currentLength = Length();

		TASSERT(a_iLength >= 0, "Length can't be less than 0");
		TASSERT(a_iLength <= 0xFFFFFF, "Too big string");

		if (a_iLength != currentLength)
		{
			if (a_iLength == 0)
			{
				if (freeMemory) TFree(m_pBuffer);

				m_pBuffer = NullString;
				m_iExcessLen = 0;

				hasChanged = true;
			}
			else
			{
				int newExcessLen = (currentLength - a_iLength) + m_iExcessLen;

				if (newExcessLen < 0 || newExcessLen > 0xFF)
				{
					if (currentLength != 0 && freeMemory)
					{
						TFree(m_pBuffer);
					}

					m_pBuffer = (char*)TMalloc(a_iLength + 1);
					m_iExcessLen = 0;

					hasChanged = true;
				}
				else
				{
					m_iExcessLen = newExcessLen;
					hasChanged = false;
				}
			}

			m_iStrLen = a_iLength;
		}

		if (freeMemory) m_pBuffer[0] = '\0';
		return hasChanged;
	}

	TString8 TString8::Format(const char* a_pcFormat, ...)
	{
		char buffer[0x400];
		TString8 buffer2;
		va_list args;

		va_start(args, a_pcFormat);

		// TASSERT is only in T2String8
		int iResult = _vsnprintf(buffer, sizeof(buffer), a_pcFormat, args);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem")
		buffer2.Copy(buffer, -1);
		
		return buffer2;
	}

	TString8& TString8::VFormat(const char* a_pcFormat, char* a_pcArgs)
	{
		char buffer[0x400];

		// TASSERT is only in T2String8
		int iResult = _vsnprintf(buffer, sizeof(buffer), a_pcFormat, a_pcArgs);
		TASSERT(iResult != -1, "PS2/GC/X360 do not correctly support _vsnprintf, this code will cause memory to be clobbered on those platforms! Increase the size of the destination string to avoid this problem")
		Copy(buffer, -1);

		return *this;
	}

	void TString8::ForceSetData(char* a_cString, int a_iLength)
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

	int TString8::FindReverse(char a_findChar, int pos) const
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

	void TString8::Truncate(uint32_t length)
	{
		if (Length() < length)
		{
			length = Length();
		}

		char* oldBuffer = m_pBuffer;

		bool allocated = AllocBuffer(length, false);
		if (allocated)
		{
			TStringManager::String8Copy(m_pBuffer, oldBuffer, length);
		}

		m_pBuffer[length] = 0;

		if (allocated && Length() != 0)
		{
			TFree(oldBuffer);
		}
	}

	void TString8::FreeBuffer()
	{
		if (Length() != 0) TFree(m_pBuffer);
		Reset();
	}

	const char* TString8::GetString(uint32_t a_iIndex) const
	{
		TASSERT(a_iIndex >= 0 && a_iIndex <= (TINT)m_iStrLen);
		if (IsIndexValid(a_iIndex)) { return &m_pBuffer[a_iIndex]; }
		return TNULL;
	}

	TString8& TString8::Concat(const char* str, uint32_t size)
	{
		uint32_t len = (uint32_t)TStringManager::String8Length(str);
		
		if ((len < size) || (size == -1))
		{
			size = len;
		}

		uint32_t oldLength = m_iStrLen;
		char* oldString = m_pBuffer;

		bool allocated = AllocBuffer(m_iStrLen + size, false);

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
			TFree(oldString);
		}

		return *this;
	}

	int TString8::Compare(const char* a_pcString, int param_2) const
	{
		TASSERT(a_pcString != TNULL, "TCString::CompareNoCase - Passed string cannot be TNULL");
		TASSERT(IsIndexValid(0), "TCString::CompareNoCase - Index 0 is not valid");
		TASSERT(GetString() != TNULL, "TCString::CompareNoCase - String cannot be TNULL");

		if (param_2 != -1)
		{
			return strncmp(GetString(), a_pcString, param_2);
		}

		const char* str = GetString();
		char bVar1 = 0;
		char bVar4 = 0;
		while (true)
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

	int TString8::CompareNoCase(const char* a_pcString, int param_2) const
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

	TString8 TString8::Mid(int param_1, int param_2) const
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

	TString8& TString8::Concat(const TString16& str, uint32_t size)
	{
		uint32_t len = str.Length();

		if ((len < size) || (size == -1))
		{
			size = len;
		}

		uint32_t oldLength = m_iStrLen;
		char* oldString = m_pBuffer;
		
		bool allocated = AllocBuffer(m_iStrLen + size, false);
		
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
			TFree(oldString);
		}

		return *this;
	}

	bool TString8::IsAllLowerCase() const
	{
		if (m_iStrLen != 0)
		{
			char* iter = m_pBuffer;
			char* endStr = m_pBuffer + m_iStrLen;

			do
			{
				char currentC = *iter++;
				if (currentC > '@' && currentC < '[') return false;

			} while (iter < endStr);
		}

		return true;
	}

	bool TString8::IsAllUpperCase() const
	{
		if (m_iStrLen != 0)
		{
			char* iter = m_pBuffer;
			char* endStr = m_pBuffer + m_iStrLen;

			do
			{
				char currentC = *iter++;
				if (currentC > '`' && currentC < '{') return false;

			} while (iter < endStr);
		}

		return true;
	}
}