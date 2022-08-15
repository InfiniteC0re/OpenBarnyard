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

	TASSERT(a_sName->Length() > 0);
	ValidateSystemPath();
	int idx = a_sName->Find(':', 0);
	if (-1 < idx)
	{
		str = TCString(0);
		str2 = TCString(0);
		str.Copy(a_sName, idx);
		const char * str3 = a_sName->GetString(idx + 1);
		str2.Copy(str3, -1);
	}
	return nullptr;
}

void __thiscall Toshi::TFileManager::ValidateSystemPath()
{
	//TODO
}
