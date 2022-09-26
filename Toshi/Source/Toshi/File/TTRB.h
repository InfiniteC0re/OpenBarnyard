#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"
#include "Toshi/Toshi2/T2String8.h"

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
			char m_Unused[2];  // Padding 0x0
			short m_Unk1;      // ? 0x2
			uint32_t m_Size;   // Size of section 0x4
			void* m_pData;     // 0x8
			uint32_t m_Unk2;   // 0xC
		};
		
		struct Header
		{
			uint32_t m_ui32Version;     // 0x0
			uint32_t m_i32SectionCount; // 0x4
		};

#pragma pack(pop)

		struct RELCEntry
		{
			short HDRX1;
			short HDRX2;
			uint32_t Offset;
		};

		struct SectionFORM
		{
			uint32_t Unk;
		};

		struct SYMBEntry
		{
			uint16_t HDRX;
			uint16_t Type_Offset;
			uint16_t padding;
			uint16_t Type_Hash;
			uint32_t Data_Offset;
		};

		struct SYMB
		{
			uint32_t m_i32SymbCount;
		};

	public:
		bool LoadTrb(const char*);
		bool ReadTrb(TTSF& ttsf);
		char* GetSymb(const char* symbName);
		int GetSymbFromSect(const char* symbName);

	private:
		Header* m_pHeader;          // 0x0
		SYMB* m_SYMB;        // 0x4
	};
}


