#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"
#include "Toshi/File/TTSF.h"

namespace Toshi
{
	// sizeof(TTRB) should be equal to 292 (de Blob) in x86 mode

	class TTRB
	{
	public:
		enum TTRB_ERROR
		{
			ERROR_OK = 0,
			ERROR_WRONG_MAGIC = 4,
			ERROR_NO_FILE = 6,
			ERROR_NOT_TRB = 7,
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

	public:
		bool LoadTrb(const char*);
		int ReadTrb(TTSF&);

	private:
		Header* m_pHeader;          // 0x0
	};
}


