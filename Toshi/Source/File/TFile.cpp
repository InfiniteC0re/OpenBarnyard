#include "ToshiPCH.h"
#include "TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	TFile::TFile(TFileSystem* pFS)
	{
		m_pFileSystem = pFS;
	}

	TFile::TFile(const TFile& other)
	{
		m_pFileSystem = other.GetFileSystem();
	}

	TFile* TFile::Create(const TString8& filename, TFILEMODE mode)
	{
		TASSERT(filename.IsIndexValid(0), "filename is empty");

		TFile* pFile = TFileManager::GetSingletonSafe()->CreateFile(filename, mode);

		if (pFile != TNULL)
		{
			TDEBUG("TFile::Create - Created file %s with mode %hu\n", filename.GetString(), mode);
		}
		else
		{
			TDEBUG("TFile::Create - Failed to create file %s with mode %hu\n", filename.GetString(), mode);
		}

		return pFile;
	}

	TString8 TFile::ConcatPath(const TString8& a_rcString, const TString8& a_rcString2)
	{
		TIMPLEMENT();
		return { };
	}

}
