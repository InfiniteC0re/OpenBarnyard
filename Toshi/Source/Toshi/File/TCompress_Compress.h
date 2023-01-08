#pragma once
#include "TFile.h"
#include "Toshi/File/TCompress.h"

namespace Toshi
{
	class TCompress_Compress
	{
	public:
		static constexpr int maxlength = 0x4000;
		static int usemaxoffset;

	public:
		static size_t Compress(TFile* file, char* data, uint32_t size, uint32_t unused, bool isBigEndian);

	private:
		static int WriteOffset(uint32_t length, int offset, char*& data, TFile* file);
		static int Write(uint32_t length, char*& data, TFile* file);
	};
}


