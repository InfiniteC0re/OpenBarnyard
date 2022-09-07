#include "ToshiPCH.h"
#include "TTSF.h"
#include "TTRB.h"

namespace Toshi
{
	uint8_t TTSF::ReadFile(TFile* a_pFile)
	{
		// FUN_00686920

		m_pFile = a_pFile;
		m_FileInfoCount = 0;

		if (m_pFile == TNULL)
		{
			return TTRB::ERROR_NO_FILE;
		}

		m_pFile->Read(&m_Header, sizeof(TTSF::Header));

		if (m_Header.m_Magic == TMAKEFOUR("TSFL"))
		{
			m_iEndianess = Endianess::Little;
		}
		else
		{
			if (m_Header.m_Magic != TMAKEFOUR("TSFB"))
			{
				return TTRB::ERROR_NOT_TRB;
			}

			m_iEndianess = Endianess::Big;
		}

		m_ReadPos += m_pFile->Read(&m_TRBF, 4);

		if (m_iEndianess == Endianess::Big)
		{
			m_Header.m_FileSize = BIG_ENDIAN_TO_LITTLE_UINT32(m_Header.m_FileSize);
			m_TRBF = BIG_ENDIAN_TO_LITTLE_UINT32(m_TRBF);
		}

		m_FileMagic = m_Header.m_Magic;
		m_FileSize = m_Header.m_FileSize;
		SaveFileInfo();

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::SaveFileInfo()
	{
		// FUN_00686130

		if (m_FileMagic != TMAKEFOUR("FORM") && m_FileMagic != TMAKEFOUR("TSFL") && m_FileMagic != TMAKEFOUR("TSFB"))
		{
			return TTRB::ERROR_WRONG_MAGIC;
		}

		m_FileInfo[m_FileInfoCount].FileStartOffset = m_pFile->Tell() - 4;
		m_FileInfo[m_FileInfoCount].FileSize = m_FileSize;
		m_FileInfoCount++;

		return TTRB::ERROR_OK;
	}
}