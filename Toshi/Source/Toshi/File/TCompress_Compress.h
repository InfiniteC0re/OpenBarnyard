#pragma once
#include "TFile.h"

namespace Toshi
{
	class TCompress_Compress
	{
		const int maxlength = 0x4000;
		static int usemaxoffset;

	public:
		int FUN_0068a830(int length, char* data, TFile* file);

		void Compress(TFile* file, char* unk, uint32_t unk2, uint32_t unk3, uint8_t endian);
	};
}


