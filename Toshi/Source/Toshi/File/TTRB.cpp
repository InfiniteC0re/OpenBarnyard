#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

namespace Toshi
{
	bool TTRB::LoadTrb(const char* fn)
	{
		TTSF ttsf;

		TCString str = TCString(fn);
		auto fs = Toshi::TFileSystem::CreateNative(str);
		
		Toshi::TFile* file = fs->CreateFile(str, 1);
		ttsf.ReadFile(file);

		return false;
	}
}
