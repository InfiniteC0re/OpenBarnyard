#include "pch.h"
#include "TFile.h"

namespace Toshi
{
	TFile::TFile(TFileSystem* param_1)
	{
		fileSystem = param_1;
	}

	TFile::TFile()
	{
	}

	TFile* TFile::Create(const TCString& path, unsigned int param_2)
	{
		TFile* file = TFileManager::Instance().CreateFile(path, param_2);
		return nullptr;
	}

	TFile* TFileManager::CreateFile(const TCString& a_sName, unsigned int param_2)
	{
		TCString str;
		TCString str2;

		TASSERT(a_sName.Length() > 0, "Name can't be empty");
		ValidateSystemPath();
		int idx = a_sName.Find(':', 0);
		if (-1 < idx)
		{
			str.Copy(a_sName, idx);
			const char * str3 = a_sName.GetString(idx + 1);
			str2.Copy(str3, -1);
		}

		return nullptr;
	}

	void TFileManager::ValidateSystemPath()
	{
		//TODO
	}
}
