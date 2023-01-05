#pragma once
#include "Toshi/File/TFile.h"
#include "TCompress.h"

namespace Toshi
{
	class TCompress_Decompress
	{
	public:
		static uintptr_t Decompress(TFile* file, TCompress::Header* header, char* buffer, uint32_t bufferSize);
		static int8_t GetHeader(TFile* file, TCompress::Header& btecHeader);
		static int GetCommand(TFile* file, bool& hasOffset, uint32_t& size, int& offset);
	};
}