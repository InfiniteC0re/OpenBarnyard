#include "ToshiPCH.h"
#include "TFile.h"
#include "Thread/T2MutexLock.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

namespace Toshi {

	TFileManager::TFileManager() :
		m_WorkingDirectory("/"),
		m_ValidatedCount(0)
	{
		InvalidateSystemPath();
	}

	void TFileManager::Destroy()
	{
		T2MutexLock mutexLock(m_Mutex);

		{
			auto pFileSystem = TFileManager::FindFileSystem("mem");
			if (pFileSystem) delete pFileSystem;
		}

		{
			auto pFileSystem = TFileManager::FindFileSystem("null");
			if (pFileSystem) delete pFileSystem;
		}

		{
			auto pFileSystem = TFileManager::FindFileSystem("abs");
			if (pFileSystem) delete pFileSystem;
		}

		{
			auto pFileSystem = TFileManager::FindFileSystem("local");
			if (pFileSystem) delete pFileSystem;
		}

		DestroyCommon();
	}

	void TFileManager::MountFileSystem(TFileSystem* a_pFileSystem)
	{
		// FUN_00685860
		T2MutexLock mutexLock(m_Mutex);
		TASSERT(TFileManager::FindFileSystem(a_pFileSystem->GetName()) == TNULL);
		TASSERT(!a_pFileSystem->IsLinked());

		m_Invalidated.InsertTail(a_pFileSystem);
		InvalidateSystemPath();
	}

	TFileSystem* TFileManager::FindFileSystem(const TString8& name)
	{
		// FUN_00685cc0
		TFileSystem* pFileSystem = TFileManager::FindFileSystem(m_Validated, name);

		if (pFileSystem == TNULL)
		{
			pFileSystem = TFileManager::FindFileSystem(m_Invalidated, name);
		}

		return pFileSystem;
	}

	TFileSystem* TFileManager::FindFileSystem(TDList<TFileSystem>& a_rList, const TString8& name)
	{
		T2_FOREACH(a_rList, pFileSystem)
		{
			if (pFileSystem->GetName() == name)
			{
				return pFileSystem;
			}
		}

		return TNULL;
	}

	TFile* TFileManager::CreateFile(const TString8& a_sName, TFILEMODE flags)
	{
		T2MutexLock mutexLock(m_Mutex);
		TASSERT(a_sName.Length() > 0, "Name can't be empty");

		ValidateSystemPath();
		TINT pos = a_sName.Find(':', 0);

		if (pos >= 0)
		{
			TString8 fsName;
			TString8 fileName;

			fsName.Copy(a_sName, pos);
			fileName.Copy(a_sName.GetString(pos + 1), -1);

			TFileSystem* pFileSystem = FindFileSystem(fsName);

			if (pFileSystem != TNULL)
			{
				return pFileSystem->CreateFile(fileName, flags);
			}
		}

		T2_FOREACH(m_Validated, pFileSystem)
		{
			TFile* pFile = pFileSystem->CreateFile(a_sName, flags);

			if (pFile != TNULL)
			{
				return pFile;
			}
		}

		return TNULL;
	}

	void TFileManager::ValidateSystemPath()
	{
		if (!m_IsValidated)
		{
			// Move all file systems to the invalidated list
			for (auto pFS = m_Validated.Tail(); pFS != m_Validated.End(); pFS = m_Validated.Tail())
			{
				pFS->Remove();
				m_Invalidated.InsertHead(pFS);
			}

			m_ValidatedCount = 0;

			TString8 fsName;
			TSysPathIter pathIter(m_SysPath);

			TBOOL hasPath = pathIter.First(fsName);

			while (hasPath)
			{
				TFileSystem* pFS = FindFileSystem(m_Invalidated, fsName);

				if (pFS != TNULL)
				{
					pFS->Remove();
					m_Validated.InsertTail(pFS);
					m_ValidatedCount += 1;
				}

				hasPath = pathIter.Next(fsName);
			}

			m_IsValidated = TTRUE;
		}
	}

	TFileManager::TSysPathIter::TSysPathIter(const TString8& str) :
		m_String(str),
		m_Position(-1)
	{

	}

	TFileManager::TSysPathIter::TSysPathIter(const TSysPathIter& other) :
		m_String(other.m_String),
		m_Position(other.m_Position)
	{

	}

	TBOOL TFileManager::TSysPathIter::First(TString8& path)
	{
		if (m_String.Length() > 0)
		{
			m_Position = m_String.Find(';', 0);
			path.Copy(m_String, m_Position);

			return TTRUE;
		}
		else
		{
			m_Position = -1;
			return TFALSE;
		}
	}

	TBOOL TFileManager::TSysPathIter::Next(TString8& path)
	{
		if (m_Position >= 0)
		{
			TINT strStart = m_Position + 1;
			m_Position = m_String.Find(';', strStart);

			path.Copy(
				m_String.GetString(strStart),
				(m_Position >= 0) ? (m_Position - strStart) : -1
			);

			return TTRUE;
		}
		else
		{
			return TFALSE;
		}
	}

	TString8 TFileManager::MakeAbsolutePath(const TString8& a_cString) const
	{
		return TFile::ConcatPath(a_cString, m_WorkingDirectory);
	}

	void TFileManager::CreateCommon()
	{
		TFileManager::CreateSingleton()->m_Mutex.Create();
	}

	void TFileManager::DestroyCommon()
	{
		delete TFileManager::GetSingletonSafe();
	}

}