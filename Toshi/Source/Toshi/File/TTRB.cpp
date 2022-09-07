#include "ToshiPCH.h"
#include "TTRB.h"

namespace Toshi
{
	bool TTRB::LoadTrb(const char* fn)
	{
		TCString str = TCString(fn);
		auto fs = Toshi::TFileSystem::CreateNative(str);
		
		Toshi::TFile* file = fs->CreateFile(str, 1);
		ReadFile(file);

		return false;
	}

	TTRB::Error TTRB::ReadFile(TFile* a_pFile)
	{
		// FUN_00686920

		m_pFile = a_pFile;
		m_FileInfoCount = 0;
		
		if (m_pFile == TNULL)
		{
			return Error::NoFile;
		}
		
		m_pFile->Read(&m_Header, sizeof(TTRB::Header));

		if (m_Header.m_Magic == TMAKEFOUR("TSFL"))
		{
			m_iEndianess = Endianess::Little;
		}
		else
		{
			if (m_Header.m_Magic != TMAKEFOUR("TSFB"))
			{
				return Error::NotTRB;
			}

			m_iEndianess = Endianess::Big;
		}

		m_Unk += m_pFile->Read(&m_TRBF, 4);

		if (m_iEndianess == Endianess::Big)
		{
			m_Header.m_FileSize = BIG_ENDIAN_TO_LITTLE_UINT32(m_Header.m_FileSize);
			m_TRBF = BIG_ENDIAN_TO_LITTLE_UINT32(m_TRBF);
		}

		m_FileMagic = m_Header.m_Magic;
		m_FileSize = m_Header.m_FileSize;
		SaveFileInfo();

		return Error::Success;
	}

	TTRB::Error TTRB::SaveFileInfo()
	{
		// FUN_00686130

		if (m_FileMagic != TMAKEFOUR("FORM") && m_FileMagic != TMAKEFOUR("TSFL") && m_FileMagic != TMAKEFOUR("TSFB"))
		{
			return Error::WrongMagic;
		}

		m_FileInfo[m_FileInfoCount].FileStartOffset = m_pFile->Tell() - 4;
		m_FileInfo[m_FileInfoCount].FileSize = m_FileSize;
		m_FileInfoCount++;

		return Error::Success;
	}
}
