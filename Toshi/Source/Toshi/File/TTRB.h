#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TTRB
	{
		enum class Endian
		{
			Little,
			Big,
		};

		struct TSF
		{
			uint32_t m_magic;
			uint32_t size;
		};

		struct SecInfo // Size 0x10
		{

		};

		struct Header
		{
			uint32_t m_ui32Version; // 0x0
			uint32_t m_i32SectionCount; // 0x4
			SecInfo* m_pSecInfo; // Name not confirmed
		};

		Header* m_pHeader; // 0x0
		Endian m_iEndianess; // 0 = Little / 1 = Big
	public:
		bool LoadTrb(const char*);
		void FUN_00686920(TFile* a_pFile);
	};
}


