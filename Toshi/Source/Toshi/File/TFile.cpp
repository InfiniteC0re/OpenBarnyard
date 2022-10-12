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

	TFile* TFile::Create(const TCString& filename, uint32_t flags)
	{
		TASSERT(filename.IsIndexValid(0), "filename is empty");

		TFile* pFile = TFileManager::Instance().CreateFile(filename, flags);

		if (pFile != TNULL)
		{
			TOSHI_CORE_TRACE("TFile::Create - Created file {0} with mode {1}", filename, flags);
			return pFile;
		}
		else
		{
			TOSHI_CORE_TRACE("TFile::Create - Failed to create file {0} with mode {1}", filename, flags);
			return TNULL;
		}
	}

	TCString TFile::ConcatPath(const TCString& a_rcString, const TCString& a_rcString2)
	{
		TIMPLEMENT();
		return { };
	}

	void TFileManager::Destroy()
	{
		TIMPLEMENT();
	}

	void TFileManager::MountFileSystem(TFileSystem* a_pFileSystem)
	{
		// FUN_00685860
		TASSERT(TFileManager::FindFileSystem(a_pFileSystem->GetName()) == TNULL, "This TFileSystem ({0}) is already mounted", a_pFileSystem->GetName());
		TASSERT(!a_pFileSystem->IsLinked(), "TFileSystem shouldn't be linked");
		
		m_FSList2.InsertTail(a_pFileSystem);
	}

	TFileSystem* TFileManager::FindFileSystem(const TCString& name)
	{
		// FUN_00685cc0
		TFileSystem* pFileSystem = TFileManager::FindFileSystem(m_FSList1, name);

		if (pFileSystem == TNULL)
		{
			pFileSystem = TFileManager::FindFileSystem(m_FSList2, name);
		}

		return pFileSystem;
	}

	TFileSystem* TFileManager::FindFileSystem(TDList<TFileSystem>& list, const TCString& name)
	{
		auto pNode = list.GetFirst();

		while (true)
		{
			if (pNode == list.GetRoot()) break;
			if (pNode->GetName() == name) return pNode;

			pNode = pNode->NextNode();
		}

		return TNULL;
	}

	TFile* TFileManager::CreateFile(const TCString& a_sName, uint32_t flags)
	{
		// FUN_006854c0
		TASSERT(a_sName.Length() > 0, "Name can't be empty");

		ValidateSystemPath();
		int pos = a_sName.Find(':', 0);

		if (pos >= 0)
		{
			TCString str1;
			TCString str2;

			str1.Copy(a_sName, pos);
			str2.Copy(a_sName.GetString(pos + 1), -1);
			TFileSystem* fs = FindFileSystem(str1);

			if (fs != TNULL)
			{
				return fs->CreateFile(str2, flags);
			}
		}

		auto pNode = m_FSList1.GetFirst();

		while (true)
		{
			if (pNode == m_FSList1.GetRoot()) break;

			TFile* pFile = pNode->CreateFile(a_sName, flags);
			if (pFile != TNULL) return pFile;
			pNode = pNode->NextNode();
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
		auto len = m_sSysPath.Length();
		
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
		m_Name = TCString(param_1.GetName());
		m_Prefix = TCString(param_1.GetPrefix());
	}

	Toshi::TFileSystem::TFileSystem(const char* a_name) : m_Name(a_name), m_Prefix()
	{
		//???
		// m_unk = m_unk
		m_handle = NULL;
	}

	Toshi::TFileSystem::~TFileSystem()
	{
		TIMPLEMENT();
	}

	void Toshi::TFileSystem::SetPrefix(TCString const& a_prefix)
	{
		m_Prefix = a_prefix;
		uint32_t len = m_Prefix.Length();
		uint32_t i = 0;

		if (0 < len)
		{
			do
			{
				if (m_Prefix[i] == '/')
				{
					m_Prefix[i] = '\\';
				}
				i++;
			} while (i < len);
		}

	}
	TFileSystem& TFileSystem::operator=(TFileSystem& a_rFileSystem)
	{
		m_Name = a_rFileSystem.GetName();
		m_Prefix = a_rFileSystem.GetPrefix();
		return *this;
	}

	STL::Ref<TFileSystem> TFileSystem::CreateNative(const char* name)
	{
#ifdef TOSHI_PLATFORM_WINDOWS
		return STL::CreateRef<TNativeFileSystem>(name);
#endif
	}
}
