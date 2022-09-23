#pragma once
#include "Toshi/File/TFile.h"
#include "Toshi/File/TCompress.h"
namespace Toshi
{
	namespace TCompress_Decompress
	{
		struct Header
		{
			uint32_t Magic;
			uint32_t Version;
			uint32_t UncompressedSize;
			uint32_t CompressedSize;
		};
	
		uintptr_t Decompress(TFile* file, Header* header, void* buffer, uint32_t& size);
		int8_t ReadHeader(TFile* file, Header& btecHeader);
		int GetReadCount(TFile* file, uint32_t& read_dst, uint32_t& size, int& offset);
	};
}