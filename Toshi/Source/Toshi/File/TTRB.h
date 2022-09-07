#pragma once
#include "Toshi/Strings/TCString.h"
#include "Toshi/File/TFile.h"

namespace Toshi
{
	// sizeof(TTRB) should be equal to 276 (de Blob)

	class TTRB
	{
		enum class Endianess
		{
			Little,
			Big,
		};

		enum class Error
		{
			Success = 0,
			WrongMagic = 4,
			NoFile = 6,
			NotTRB = 7,
		};

		struct Header
		{
			uint32_t m_Magic;
			uint32_t m_FileSize;
		};

		struct SecInfo // Size 0x10
		{

		};

		// idk how to call it and what's it's purpose
		struct FileInfo
		{
			uint32_t FileStartOffset; // offset to TRBF
			uint32_t FileSize;        // just the size
		};

		//struct Header
		//{
		//	uint32_t m_ui32Version; // 0x0
		//	uint32_t m_i32SectionCount; // 0x4
		//	SecInfo* m_pSecInfo; // Name not confirmed
		//};

	public:
		bool LoadTrb(const char*);
		Error ReadFile(TFile* a_pFile);
		Error SaveFileInfo();

	private:
		// Header* m_pHeader;          // 0x0 (it's not from TTRB)
		Endianess m_iEndianess;     // 0x0
		TFile* m_pFile;             // 0x4
		uint32_t m_FileInfoCount;   // 0x8
		FileInfo m_FileInfo[32];    // 0xC
		Header m_Header;            // 0x10C
		uint32_t m_TRBF;            // 0x114
		uint32_t m_FileMagic;       // 0x118
		uint32_t m_FileSize;        // 0x11C
		uint32_t m_Unk;             // 0x120
	};
}


