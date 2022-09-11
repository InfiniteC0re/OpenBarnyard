#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	// sizeof(TTRB) should be equal to 292 (de Blob) in x86 mode
	class TTSF;

	class TTRB
	{
	public:
		enum TTRB_ERROR : uint8_t
		{
			ERROR_OK = 0,
			ERROR_WRONG_MAGIC = 4,
			ERROR_FORM_MAGIC = 5,
			ERROR_NO_FILE = 6,
			ERROR_NOT_TRB = 7,
			ERROR_NO_FILEINFO_ON_STACK = 8,
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

		struct RELCEntry
		{

		};

		struct SectionFORM
		{
			uint32_t Unk;
		};

	public:
		bool LoadTrb(const char*);
		bool ReadTrb(TTSF& ttsf);

	private:
		Header* m_pHeader;          // 0x0
		uint32_t m_SYMBSize;        // 0x4
	};
}


