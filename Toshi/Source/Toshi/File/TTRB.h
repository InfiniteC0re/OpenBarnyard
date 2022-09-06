#pragma once

#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"
#include "Toshi/File/TNativeFileSystem.h"
#include "Toshi/File/TNativeFile.h"

namespace Toshi
{
	class TTRB
	{
		enum Endian
		{
			LITTLE,
			Big
		};

		struct TSF
		{
			int m_magic;
			int size;
		};

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
		Endian m_iEndianess; // 0 = Little / 1 = Big
	public:
		bool LoadTrb(const char*);
		void FUN_00686920(TNativeFile* a_pFile);
	};
}


