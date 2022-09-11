#include "ToshiPCH.h"
#include "TModel.h"

bool Toshi::TModel::LoadTMD(const char* a_tmdPath)
{
	auto fs = Toshi::TFileSystem::CreateNative("local");
	auto file = fs->CreateFile(a_tmdPath, Toshi::TFile::OpenFlags_Read);

	Header header;

	if (file != TNULL)
	{
		file->Read(&header, sizeof(Header));
		if (header.m_magic == TMAKEFOUR("TMDL"))
		{

		}
	}
	return false;
}

bool Toshi::TModel::LoadTRBTMD(const char*)
{

	return false;
}
