#include "pch.h"
#include "TFile.h"

namespace Toshi
{
	TFile::TFile(TFileSystem* param_1)
	{
		m_pFileSystem = param_1;
	}

	/*TFile::TFile()
	{
	}*/

	TFile::TFile(const TFile& a_pFile)
	{
		m_pFileSystem = a_pFile.GetFileSystem();
	}

	TFile* TFile::Create(const TCString& path, unsigned int param_2)
	{
		TFile* file = TFileManager::Instance().CreateFile(path, param_2);
		return file;
	}

	void TFileManager::MountFileSystem(TFileSystem* a_pFileSystem)
	{
	}

	TFileSystem* TFileManager::FindFileSystem(const TCString&)
	{
		return nullptr;
	}

	TFile* TFileManager::CreateFile(const TCString& a_sName, unsigned int param_2)
	{
		TCString str = TCString();
		TCString str2 = TCString();

		TASSERT(a_sName.Length() > 0, "Name can't be empty");
		ValidateSystemPath();
		int idx = a_sName.Find(':', 0);
		if (-1 < idx)
		{
			str.Copy(a_sName, idx);
			const char * str3 = a_sName.GetString(idx + 1);
			str2.Copy(str3, -1);
			TFileSystem* fs = FindFileSystem(a_sName);
			if (fs != TNULL)
			{

			}

		}

		return nullptr;
	}

	void TFileManager::ValidateSystemPath()
	{
		//TODO
	}
}
