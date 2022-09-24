#pragma once
#include "Toshi/File/TFile.h"
#include "Toshi/File/TCompress.h"

namespace Toshi
{
	class TCompress_Decompress
	{
	public:
		struct Header
		{
			uint32_t Magic;
			uint32_t Version;
			uint32_t UncompressedSize;
			uint32_t CompressedSize;
			uint32_t XorValue;
		};
	
		static uintptr_t Decompress(TFile* file, Header* header, void* buffer, uint32_t& size);
		static int8_t ReadHeader(TFile* file, Header& btecHeader, uint16_t& headerSize);
		static int GetReadCount(TFile* file, bool& hasOffset, uint32_t& size, int& offset);
	};
}