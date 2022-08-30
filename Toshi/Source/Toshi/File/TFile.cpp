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
	
	Toshi::TFileSystem::TFileSystem(TFileSystem const& param_1)
	{
		m_unk = param_1.m_unk;
		m_unk2 = param_1.m_unk2;
		m_sName = TCString(param_1.GetName());
		m_prefix = TCString(param_1.GetPrefix());
	}

	Toshi::TFileSystem::TFileSystem(const char* a_name)
	{
		//???
		// m_unk = m_unk
		m_unk = TNULL;
		m_unk2 = m_unk;

		m_sName = TCString(a_name);
		m_prefix = TCString();
	}

	Toshi::TFileSystem::~TFileSystem()
	{
		m_sName.~TCString();
		m_prefix.~TCString();
	}

	void Toshi::TFileSystem::SetPrefix(TCString const& a_prefix)
	{
		m_prefix = a_prefix;
		uint32_t len = m_prefix.Length();
		uint32_t i = 0;

		if (0 < len)
		{
			do
			{
				if (m_prefix[i] == '/')
				{
					m_prefix[i] = '\\';
				}
				i++;
			} while (i < len);
		}

	}
}
