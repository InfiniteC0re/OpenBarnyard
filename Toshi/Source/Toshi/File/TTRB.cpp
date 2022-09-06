#include "ToshiPCH.h"
#include "TTRB.h"

bool Toshi::TTRB::LoadTrb(const char* fn)
{
	//TFile* f = TFile::Create(fn, 1);

	TCString str = TCString(fn);
	Toshi::TNativeFileSystem * fs = Toshi::tnew<Toshi::TNativeFileSystem>(str);
	Toshi::TNativeFile* f = (Toshi::TNativeFile*)fs->CreateFile(str, 1);
	FUN_00686920(f);
	return false;
}

void Toshi::TTRB::FUN_00686920(TNativeFile* a_pFile)
{
	TSF tsf;
	a_pFile->Read(&tsf, 8);

	if (tsf.m_magic == 0x4C465354)
	{
		m_iEndianess = Endian::LITTLE;
	}
	else if (tsf.m_magic == 0x42465354)
	{
		m_iEndianess = Endian::Big;
	}
	else
	{
		TOSHI_ERROR("Not a Toshi File");
	}

}
