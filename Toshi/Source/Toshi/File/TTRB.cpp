#include "ToshiPCH.h"
#include "TTRB.h"

bool Toshi::TTRB::LoadTrb(const char* fn)
{
	//TFile* f = TFile::Create(fn, 1);

	TCString str = TCString(fn);
	auto fs = Toshi::TFileSystem::CreateNative(str);
	Toshi::TFile* file = fs->CreateFile(str, 1);
	FUN_00686920(file);
	return false;
}

void Toshi::TTRB::FUN_00686920(TFile* a_pFile)
{
	TASSERT(a_pFile != TNULL, "a_pFile is TNULL")
	TSF tsf;
	a_pFile->Read(&tsf, sizeof(TSF));

	if (tsf.m_magic == TMAKEFOUR("TSFL"))
	{
		m_iEndianess = Endian::Little;
	}
	else if (tsf.m_magic == TMAKEFOUR("TSFB"))
	{
		m_iEndianess = Endian::Big;
	}
	else
	{
		TOSHI_ERROR("Not a Toshi File");
	}
}
