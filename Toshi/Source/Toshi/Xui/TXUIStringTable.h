#pragma once

namespace Toshi
{
	class TXUIStringTable
	{
	public:
		static const uint32_t IDXUS = TMAKEFOUR("XUIS");

		~TXUIStringTable();

		struct Str
		{
			int value;
			wchar_t* string;
		};

		Toshi::TXUIStringTable::Str* Lookup(const wchar_t* a_str);

		struct XUSHeader
		{
			uint32_t m_uiFileID;
		};

		XUSHeader m_oXUSHeader; // 0x0
		size_t m_uiCountOfStrings; // 0xC
		Str** m_aStringTable; // 0x10
	};
}


