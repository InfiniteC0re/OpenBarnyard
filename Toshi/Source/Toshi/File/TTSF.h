#pragma once
#include "TTRB.h"
#include "Toshi/File/TCompress_Decompress.h"
#include "Toshi/File/TCompress_Compress.h"

namespace Toshi
{
	class TTSF
	{
	public:
		typedef uint8_t Endianess;

		enum Endianess_ : Endianess
		{
			Endianess_Little,
			Endianess_Big,
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
		inline ~TTSF() { Close(); }
		
		TTRB::ERROR Open(TFile* a_pFile);

		TTRB::ERROR PushForm();
		TTRB::ERROR PopForm();

		// 0x00688250
		inline void ReadRaw(void* dest, uint32_t size) { m_ReadPos += m_pFile->Read(dest, size); }

		// Sections related stuff
		TTRB::ERROR ReadHunk();
		TTRB::ERROR SkipHunk();
		TTRB::ERROR ReadFORM(TTRB::SectionFORM* section);
		TTRB::ERROR ReadHunkData(void* dest);

		void Close(bool free = true);

		void ReadCompressed(void* buffer, uint32_t size);
		inline void CompressSection(TFile* file, char* unk, uint32_t unk2, uint32_t unk3) { TCompress_Compress::Compress(file, unk, unk2, unk3, m_Endianess); }

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


