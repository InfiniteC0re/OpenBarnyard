#include "ToshiPCH.h"
#include "T2Locale.h"

namespace Toshi
{
	T2Locale* T2Locale::s_Singleton = TNULL;

	T2Locale::T2Locale(int langCount, size_t bufferSize, void* buffer)
	{
		// 00662d40
		m_Buffer = buffer;
		m_BufferPos = buffer;
		m_StringTable = nullptr;
		m_LangId = -1;
		m_Flag = false;

		if (m_Buffer == nullptr)
		{
			size_t symbolTableSize = TTRB::GetSymbolTableSize(1);
			size_t headerSize = TTRB::GetHeaderSize(1);
			m_Buffer = TMalloc(headerSize + bufferSize + symbolTableSize);
			m_BufferPos = m_Buffer;
			m_Flag = true;
		}

		m_Locale.SetMemoryFunctions(TRBAllocator, TRBDeallocator, this);
	}

	void T2Locale::SetLanguage(Lang langid)
	{
		// 00662e30
		if (langid != m_LangId && (m_Locale.Close(), -1 < langid))
		{
			m_Locale.Load(GetLanguageFilename(langid));
			m_StringTable = m_Locale.CastSymbol<LocaleStrings>("LocaleStrings");

			m_BufferPos = m_Buffer;
			m_LangId = langid;
		}
	}
}
