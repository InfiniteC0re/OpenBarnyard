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
		};
	
		static TCompress_ERROR Decompress(TFile* file, Header* header, void* buffer, uint32_t size);
		static TCompress_ERROR ReadHeader(TFile* file, Header& btecHeader);
	private:
		int GetReadCount();
	};
}