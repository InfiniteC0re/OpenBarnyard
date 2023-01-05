#pragma once
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TCompress_Decompress
	{
	public:
		struct Header
		{
			uint32_t Magic;
			TVersion Version;
			uint32_t CompressedSize;
			uint32_t Size;
			uint32_t XorValue;
		};

		static uintptr_t Decompress(TFile* file, Header* header, char* buffer, uint32_t bufferSize);
		static int8_t GetHeader(TFile* file, Header& btecHeader);
		static int GetCommand(TFile* file, bool& hasOffset, uint32_t& size, int& offset);

	public:
		// Size of header that is common for any BTEC version
		static constexpr uint32_t HEADER_SIZE_COMMON = sizeof(Header) - sizeof(Header::XorValue);

		// Size of header that is common for BTEC 1.3
		static constexpr uint32_t HEADER_SIZE_13 = sizeof(Header);
	};
}