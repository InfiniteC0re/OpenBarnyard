#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

namespace Toshi
{
	bool TTRB::LoadTrb(const char* fn)
	{
		TTSF ttsf = TTSF();

		TCString str = TCString(fn);
		auto fs = Toshi::TFileSystem::CreateNative(str);
		
		Toshi::TFile* file = fs->CreateFile(str, 1);
		int error = ttsf.ReadFile(file);

		if (error == TTRB_ERROR::ERROR_OK)
		{
			ReadTrb(ttsf);
		}

		return false;
	}

	int TTRB::ReadTrb(TTSF& param_1)
	{
		int iVar11 = param_1.m_FileSize - 4;
		const int size = 0x200;

		if (iVar11 < 1) 
		{
			param_1.FUN_006881B0();
			return 0; //idk
		}

		param_1.SaveFileInfo();
		return 0;
	}
}
