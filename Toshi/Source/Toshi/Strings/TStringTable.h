#pragma once

namespace Toshi
{
	class TStringTable
	{
	public:
		TStringTable(const char* const* strings, int numStrings)
		{
			m_Strings = strings;
			m_NumStrings = numStrings;
		}
		
		const char* GetStringFromID(int id)
		{
			TASSERT(id >= 0 && id < m_NumStrings);
			return m_Strings[id];
		}

		int GetStringID(const char* string);

	private:
		const char* const* m_Strings;
		int m_NumStrings;
	};
}