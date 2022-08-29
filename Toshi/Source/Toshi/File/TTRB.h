#pragma once

#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TTRB
	{
		struct SecInfo // Size 0x10
		{

		};

		struct Header
		{
			unsigned int m_ui32Version; // 0x0
			int m_i32SectionCount; // 0x4
			SecInfo* m_pSecInfo; // Name not confirmed
		};

		Header* m_pHeader; // 0x0

		bool LoadTrb(const char*);
	};
}


