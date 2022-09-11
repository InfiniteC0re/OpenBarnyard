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

		if (m_Header.Magic == TMAKEFOUR("TSFL"))
		{
			m_Endianess = Endianess::Little;
		}
		else
		{
			if (m_Header.Magic != TMAKEFOUR("TSFB"))
			{
				return TTRB::ERROR_NOT_TRB;
			}

			m_Endianess = Endianess::Big;
		}

		m_ReadPos += m_pFile->Read(&m_TRBF, 4);

		if (m_Endianess == Endianess::Big)
		{
			m_Header.FileSize = BIG_ENDIAN_TO_LITTLE_UINT32(m_Header.FileSize);
			m_TRBF = BIG_ENDIAN_TO_LITTLE_UINT32(m_TRBF);
		}

		m_CurrentSection.Name = m_Header.Magic;
		m_CurrentSection.Size = m_Header.FileSize;
		PushFileInfo();

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::PushFileInfo()
	{
		// FUN_00688160

		if (m_CurrentSection.Name != TMAKEFOUR("FORM") &&
			m_CurrentSection.Name != TMAKEFOUR("TSFL") &&
			m_CurrentSection.Name != TMAKEFOUR("TSFB"))
		{
			return TTRB::ERROR_WRONG_MAGIC;
		}

		m_FileInfo[m_FileInfoCount].FileStartOffset = m_pFile->Tell() - 4;
		m_FileInfo[m_FileInfoCount].FileSize = m_CurrentSection.Size;
		m_FileInfoCount++;

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::PopFileInfo()
	{
		// FUN_006881B0
		if (m_FileInfoCount < 1) return TTRB::ERROR_NO_FILEINFO_ON_STACK;
		
		m_FileInfoCount--;
		auto& fileInfo = m_FileInfo[m_FileInfoCount];

		m_CurrentSection.Size = fileInfo.FileSize;
		m_ReadPos = m_pFile->Tell() - fileInfo.FileStartOffset;

		uint32_t alignedPos = TMath::AlignNumUp(m_CurrentSection.Size);
		m_pFile->Seek(alignedPos - m_ReadPos, TFile::TSEEK_CUR);
		m_ReadPos = alignedPos;

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::FUN_00687FA0()
	{

		return uint8_t();
	}

	uint8_t TTSF::ReadSectionHeader()
	{
		// FUN_00687fa0
		m_pFile->Read(&m_CurrentSection, sizeof(Section));
		
		if (m_Endianess != Endianess::Little)
		{
			m_CurrentSection.Name = BIG_ENDIAN_TO_LITTLE_UINT32(m_CurrentSection.Name);
			m_CurrentSection.Size = BIG_ENDIAN_TO_LITTLE_UINT32(m_CurrentSection.Size);
		}

		m_ReadPos = 0;

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::SkipSection()
	{
		// FUN_006880e0
		uint32_t alignedSize = TMath::AlignNumUp(m_CurrentSection.Size);
		m_pFile->Seek(alignedSize, TFile::TSEEK_CUR);
		m_ReadPos = alignedSize;

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::ReadFORM(TTRB::SectionFORM* section)
	{
		// FUN_00688120
		if (m_CurrentSection.Name != TMAKEFOUR("FORM"))
		{
			return TTRB::ERROR_WRONG_MAGIC;
		}

		m_ReadPos += m_pFile->Read(section, sizeof(TTRB::SectionFORM));

		return TTRB::ERROR_OK;
	}

	uint8_t TTSF::ReadSectionData(void* dst)
	{
		if (m_CurrentSection.Name == TMAKEFOUR("FORM"))
		{
			return TTRB::ERROR_FORM_MAGIC;
		}

		TASSERT(m_ReadPos == 0, "m_ReadPos should be zero");
		m_ReadPos += m_pFile->Read(dst, m_CurrentSection.Size);

		uint32_t pos = m_pFile->Tell();

		// align position to 4
		if ((pos & 3) != 0)
		{
			char dummy[4];
			m_ReadPos += m_pFile->Read(dummy, 4 - (pos & 3));
		}

		return TTRB::ERROR_OK;
	}

	void TTSF::LogUnknownSection()
	{
		char charName[sizeof(m_CurrentSection.Name) + 1] = {};
		*charName = m_CurrentSection.Name;

		TOSHI_CORE_ERROR("Unknown TRB section: {0}", charName);
	}

	TTSF::TTSF() : m_FileInfo(), m_Header()
	{
		m_pFile = TNULL;
		m_FileInfoCount = 0;
		m_ReadPos = 0;
		// FUN_007EC9F0(m_FileInfo, 0, 0x100);
		m_TRBF = 0;
		m_CurrentSection.Name = 0;
		m_CurrentSection.Size = 0;
		m_Endianess = Endianess::Little;
	}
}