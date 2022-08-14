#include "TFile.h"

Toshi::TFile* __stdcall Toshi::TFile::Create(TCString const* param_1, unsigned int param_2)
{
	TFile* file = s_pFileManager->CreateFile(param_1, param_2);
	return nullptr;
}

Toshi::TFile* __thiscall Toshi::TFileManager::CreateFile(TCString const* a_sName, unsigned int param_2)
{
	TCString str;
	TCString str2;

	//TASSERT(a_sName.Length() > 0);
	ValidateSystemPath();
	int idx = a_sName->Find(':', 0);
	if (-1 < idx)
	{
		str = TCString();
		str2 = TCString();
		str.Copy(a_sName, idx);
	}
	return nullptr;
}

void __thiscall Toshi::TFileManager::ValidateSystemPath()
{
	//TODO
}
