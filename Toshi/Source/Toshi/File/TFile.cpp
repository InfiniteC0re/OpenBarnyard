#include "ToshiPCH.h"
#include "TFile.h"
#include "Platform/Windows/TNativeFileSystem.h"

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

	TFile* TFile::Create(const TCString& fn, uint32_t flags)
	{
		return TFileManager::Instance().CreateFile(fn, flags);
	}

	TCString TFile::ConcatPath(const TCString& a_rcString, const TCString& a_rcString2)
	{
		TIMPLEMENT();
		return TCString();
	}

	void TFileManager::MountFileSystem(TFileSystem* a_pFileSystem)
	{
		TIMPLEMENT();
	}

	TFileSystem* TFileManager::FindFileSystem(const TCString&)
	{
		TIMPLEMENT();
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

	Toshi::TFileManager::TSysPathIter::TSysPathIter(const TCString& a_sSysPath)
	{
		m_sSysPath = a_sSysPath;
		m_position = -1;
	}

	bool TFileManager::TSysPathIter::Next(TCString& param_1)
	{
		if (m_position > -1)
		{
			int oldPos = m_position + 1;
			m_position = m_sSysPath.Find(';', oldPos);

			if (m_position < 0)
			{
				param_1.Copy(m_sSysPath.GetString(oldPos), -1);
			}
			else
			{
				param_1.Copy(m_sSysPath.GetString(oldPos), m_position - oldPos);
			}
			return true;
		}
		return false;
	}

	bool TFileManager::TSysPathIter::First(TCString& param_1)
	{
		int len = m_sSysPath.Length();
		if (len > 0)
		{
			m_position = m_sSysPath.Find(';', 0);

			if (m_position < 0)
			{
				param_1.Copy(m_sSysPath, -1);
			}
			else
			{
				param_1.Copy(m_sSysPath, m_position);
			}
			return true;
		}
		m_position = -1;
		return false;
	}

	Toshi::TFileManager::TSysPathIter::TSysPathIter(const TSysPathIter& a_rSysPathIter)
	{
		m_sSysPath = a_rSysPathIter.m_sSysPath;
		m_position = a_rSysPathIter.m_position;
	}

	void TFileManager::ValidateSystemPath()
	{
		//if (t == TNULL)
	}
	
	Toshi::TFileSystem::TFileSystem(TFileSystem const& param_1)
	{
		m_unk = param_1.m_unk;
		m_unk2 = param_1.m_unk2;
		m_sName = TCString(param_1.GetName());
		m_prefix = TCString(param_1.GetPrefix());
	}

	Toshi::TFileSystem::TFileSystem(const char* a_name) : m_sName(a_name), m_prefix()
	{
		//???
		// m_unk = m_unk
		m_unk = TNULL;
		m_unk2 = m_unk;
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
	TFileSystem& TFileSystem::operator=(TFileSystem& a_rFileSystem)
	{
		m_unk = a_rFileSystem.m_unk;
		m_unk2 = a_rFileSystem.m_unk;
		m_sName = a_rFileSystem.GetName();
		m_prefix = a_rFileSystem.GetPrefix();
		return *this;
	}

	STL::Ref<TFileSystem> TFileSystem::CreateNative(const char* name)
	{
#ifdef TOSHI_PLATFORM_WINDOWS
		return STL::CreateRef<TNativeFileSystem>(name);
#endif
	}
}
