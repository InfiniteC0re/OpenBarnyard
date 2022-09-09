#pragma once
#include "Toshi/File/TFile.h"

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
			uint32_t m_Magic;
			uint32_t m_FileSize;
		};

		// idk how to call it and what's it's purpose
		struct FileInfo
		{
			uint32_t FileStartOffset; // offset to TRBF
			uint32_t FileSize;        // just the size
		};

	public:
		uint8_t ReadFile(TFile* a_pFile);
		uint8_t SaveFileInfo();
		uint8_t FUN_006881B0(); // de blob address
		uint8_t FUN_00687FA0(); // de blob address

		TTSF();

	public:
		Endianess m_iEndianess;     // 0x0
		TFile* m_pFile;             // 0x4
		uint32_t m_FileInfoCount;   // 0x8
		FileInfo m_FileInfo[32];    // 0xC
		int* m_Unk;				// 0x10
		Header m_Header;            // 0x10C
		uint32_t m_TRBF;            // 0x114
		uint32_t m_FileMagic;       // 0x118
		uint32_t m_FileSize;        // 0x11C
		uint32_t m_ReadPos;         // 0x120
	};
}


