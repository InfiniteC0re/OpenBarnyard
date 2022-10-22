#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi/Locale/T2Locale.h>

namespace Toshi
{
	class ALocaleLookup : TSingleton<ALocaleLookup>
	{
	public:
		struct LocaleStrings
		{
			T2Locale::LocaleId Count;
			char** Strings;
		};

	public:
		ALocaleLookup(const char* filename)
		{
			m_Enum.Load(filename);
			m_StringTable = m_Enum.CastSymbol<LocaleStrings>("LocaleStrings");
		}

		T2Locale::LocaleId LookUp(const char* str)
		{
			if (str != TNULL && m_StringTable != TNULL)
			{
				for (T2Locale::LocaleId i = 0; i < m_StringTable->Count; i++)
				{
					if (strcmp(m_StringTable->Strings[i], str) == 0)
					{
						return i;
					}
				}
			}

			return -1;
		}

	private:
		LocaleStrings* m_StringTable;
		Toshi::TTRB m_Enum;
	};
}