#include "ToshiPCH.h"
#include "TModel.h"

bool Toshi::TModel::LoadTMD(const char* a_tmdPath)
{
	auto file = TFile::Create(a_tmdPath, Toshi::TFile::OpenFlags_Read);

	Header header;

	if (file != TNULL)
	{
		file->Read(&header, sizeof(Header));

		if (header.m_magic == TMAKEFOUR("TMDL"))
		{

		}

		file->Destroy();
	}

	return false;
}

void Toshi::TModel::CreateResource(const char* name)
{
	T2Resource::CreateResource(name, this, TNULL, this);
	
}

bool Toshi::TModel::LoadTRBTMD(const char*)
{

	return false;
}
