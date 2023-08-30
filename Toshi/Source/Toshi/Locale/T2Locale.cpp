#include "ToshiPCH.h"
#include "T2Locale.h"

namespace Toshi
{
	T2Locale::T2Locale(int langCount, size_t bufferSize, void* buffer)
	{
		// 00662d40
		m_pBuffer = buffer;
		m_BufferPos = buffer;
		m_StringTable = TNULL;
		m_LangId = -1;
		m_bOwnsBuffer = TFALSE;

		if (m_pBuffer == TNULL)
		{
			size_t symbolTableSize = TTRB::GetSymbolTableSize(1);
			size_t headerSize = TTRB::GetHeaderSize(1);
			m_pBuffer = TMalloc(headerSize + bufferSize + symbolTableSize);
			m_BufferPos = m_pBuffer;
			m_bOwnsBuffer = TTRUE;
		}

		m_Locale.SetMemoryFunctions(TRBAllocator, TRBDeallocator, this);
	}

	T2Locale::~T2Locale()
	{
		m_Locale.Close();
		
		if (m_bOwnsBuffer)
		{
			TFree(m_pBuffer);
		}
	}

	void T2Locale::SetLanguage(Lang langid)
	{
		// 00662e30
		if (langid != m_LangId && (m_Locale.Close(), -1 < langid))
		{
			m_Locale.Load(GetLanguageFilename(langid));
			m_StringTable = m_Locale.CastSymbol<LocaleStrings>("LocaleStrings");

			m_BufferPos = m_pBuffer;
			m_LangId = langid;
		}
	}
}
