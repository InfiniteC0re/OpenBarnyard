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

#pragma pack(push, 1)
		// HDRX
		
		struct SecInfo
		{
			short m_Unk1;      // ?
			uint32_t m_Size;   // 
			void* m_pData;     // 
			uint32_t m_Unk2;   // 
			char m_Padding[2]; // 
		};
		
		struct Header
		{
			uint32_t m_ui32Version;     // 0x0
			uint32_t m_i32SectionCount; // 0x4
			char m_Unused[2];           // Padding value?
		};

#pragma pack(pop)

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


