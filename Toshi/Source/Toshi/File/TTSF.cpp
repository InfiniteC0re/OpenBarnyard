#include "ToshiPCH.h"
#include "TTSF.h"

namespace Toshi
{
	TTRB::ERROR TTSF::ReadFile(TFile* a_pFile)
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
			m_Endianess = Endianess_Little;
		}
		else
		{
			if (m_Header.Magic != TMAKEFOUR("TSFB"))
			{
				return TTRB::ERROR_NOT_TRB;
			}

			m_Endianess = Endianess_Big;
		}

		m_ReadPos += m_pFile->Read(&m_TRBF, 4);

		if (m_Endianess == Endianess_Big)
		{
			m_Header.FileSize = BIG_ENDIAN_TO_LITTLE(m_Header.FileSize);
			m_TRBF = BIG_ENDIAN_TO_LITTLE(m_TRBF);
		}

		m_CurrentSection.Name = m_Header.Magic;
		m_CurrentSection.Size = m_Header.FileSize;
		PushFileInfo();

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSF::PushFileInfo()
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

	TTRB::ERROR TTSF::PopFileInfo()
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

	TTRB::ERROR TTSF::ReadSectionHeader()
	{
		// FUN_00687fa0
		m_pFile->Read(&m_CurrentSection, sizeof(Section));
		
		if (m_Endianess != Endianess_Little)
		{
			m_CurrentSection.Name = BIG_ENDIAN_TO_LITTLE(m_CurrentSection.Name);
			m_CurrentSection.Size = BIG_ENDIAN_TO_LITTLE(m_CurrentSection.Size);
		}

		m_ReadPos = 0;

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSF::SkipSection()
	{
		// FUN_006880e0
		uint32_t alignedSize = TMath::AlignNumUp(m_CurrentSection.Size);
		m_pFile->Seek(alignedSize - m_ReadPos, TFile::TSEEK_CUR);
		m_ReadPos = alignedSize;

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSF::ReadFORM(TTRB::SectionFORM* section)
	{
		// FUN_00688120
		if (m_CurrentSection.Name != TMAKEFOUR("FORM"))
		{
			return TTRB::ERROR_WRONG_MAGIC;
		}

		m_ReadPos += m_pFile->Read(section, sizeof(TTRB::SectionFORM));

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSF::ReadSectionData(void* dst)
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

	void TTSF::Destroy(bool free)
	{
		PopFileInfo();

		if (m_pFile != TNULL && free)
		{
			m_pFile->Destroy();
		}

		m_pFile = TNULL;
		m_FileInfoCount = 0;
	}

	void TTSF::DecompressSection(void* buffer, uint32_t size)
	{
		TCompress_Decompress::Header header;
		uint32_t originalPos = m_pFile->Tell();

		int8_t error = TCompress_Decompress::ReadHeader(m_pFile, header);

		if (error == TCOMPRESS_ERROR_OK)
		{
			uint32_t headerSize = m_pFile->Tell() - originalPos;
			TCompress_Decompress::Decompress(m_pFile, &header, buffer, size);
			m_ReadPos += header.UncompressedSize + headerSize;
		}
	}

	void TTSF::LogUnknownSection()
	{
#ifdef TOSHI_DEBUG
		char charName[sizeof(m_CurrentSection.Name) + 1] = {};
		*charName = m_CurrentSection.Name;

		TOSHI_CORE_ERROR("Unknown TRB section: {0}", charName);
#endif
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
		m_Endianess = Endianess_Little;
	}
}