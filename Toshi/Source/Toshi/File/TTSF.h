#pragma once
#include "TTRB.h"
#include "Toshi/File/TCompress_Decompress.h"
#include "Toshi/File/TCompress_Compress.h"

namespace Toshi
{
	class TTSF
	{
	public:
		enum class Endianess
		{
			Little,
			Big,
		};

		struct Header
		{
			uint32_t Magic;
			uint32_t FileSize;
		};

		// idk how to call it and what's it's purpose
		struct FileInfo
		{
			uint32_t FileStartOffset; // offset to TRBF
			uint32_t FileSize;        // just the size
		};

		struct Section
		{
			uint32_t Name;
			uint32_t Size;
		};

	public:
		TTSF();
		inline ~TTSF() { Destroy(); }
		
		uint8_t ReadFile(TFile* a_pFile);
		uint8_t FUN_00687FA0(); // de blob address

		uint8_t PushFileInfo();
		uint8_t PopFileInfo();

		// 0x00688250
		inline void ReadBytes(void* dest, uint32_t size) { m_ReadPos += m_pFile->Read(dest, size); }

		// Sections related stuff
		uint8_t ReadSectionHeader();
		uint8_t SkipSection();
		uint8_t ReadFORM(TTRB::SectionFORM* section);
		uint8_t ReadSectionData(void* dest);

		void Destroy(bool free = true);

		void DecompressSection(void* buffer, uint32_t size);
		inline void CompressSection(TFile* file, char* unk, uint32_t unk2, uint32_t unk3) { TCompress_Compress::Compress(file, unk, unk2, unk3, (uint8_t)m_Endianess); }

		void LogUnknownSection();

	public:
		friend Toshi::TTRB;

	private:
		Endianess m_Endianess;     // 0x0
		TFile* m_pFile;             // 0x4
		uint32_t m_FileInfoCount;   // 0x8
		FileInfo m_FileInfo[32];    // 0xC
		Header m_Header;            // 0x10C
		uint32_t m_TRBF;            // 0x114
		Section m_CurrentSection;   // 0x118
		uint32_t m_ReadPos;         // 0x120
	};
}


