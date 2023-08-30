#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/File/TTRB.h"

namespace Toshi
{
	class T2Locale : public Toshi::TSingleton<T2Locale>
	{
	public:
		using Lang = int32_t;
		using LocaleId = int32_t;
		using LocaleString = const wchar_t*;

		struct LocaleStrings
		{
			LocaleId m_numstrings;
			LocaleString* Strings;
		};

	public:
		T2Locale(const T2Locale&) = delete;
		T2Locale(const T2Locale&&) = delete;

		virtual ~T2Locale();
		virtual const char* GetLanguageFilename(Lang lang) = 0;

		void SetLanguage(Lang langid);

	protected:
		T2Locale(int langCount, size_t bufferSize, void* buffer);

		LocaleString Get(LocaleId locid)
		{
			if (locid > -1 && locid < m_StringTable->m_numstrings)
			{
				return m_StringTable->Strings[locid];
			}

			return TNULL;
		}

		int GetNumStrings() const { return m_StringTable->m_numstrings; }
		
		static void* TRBAllocator(TTRB::AllocType alloctype, size_t size, short unk, size_t unk2, void* userData)
		{
			return static_cast<T2Locale*>(userData)->TRBAlloc(size);
		}

		static void TRBDeallocator(TTRB::AllocType alloctype, void* ptr, short unk, size_t unk2, void* userData)
		{
			// T2Locale doesn't have deallocator
		}

		void* TRBAlloc(size_t size)
		{
			void* ptr = m_BufferPos;
			m_BufferPos = static_cast<char*>(ptr) + size;
			return ptr;
		}

	protected:
		TTRB m_Locale;                // 0x04
		void* m_pBuffer;              // 0x18
		void* m_BufferPos;            // 0x1C
		LocaleStrings* m_StringTable; // 0x20
		Lang m_LangId;                // 0x24
		TBOOL m_bOwnsBuffer;          // 0x28
	};
}

