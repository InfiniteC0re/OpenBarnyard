#include "ToshiPCH.h"
#include "TTSF.h"

namespace Toshi
{
	TTRB::ERROR TTSFI::Open(TFile* a_pFile)
	{
		// FUN_00686920

		m_pFile = a_pFile;
		m_FileInfoCount = 0;

		if (m_pFile == TNULL)
		{
			return TTRB::ERROR_NO_FILE;
		}

		m_pFile->Read(&m_Header, sizeof(TTSFI::Header));

		if (m_Header.Magic == IDMAGICL)
		{
			m_Endianess = Endianess_Little;
		}
		else
		{
			if (m_Header.Magic != IDMAGICB)
			{
				return TTRB::ERROR_NOT_TRB;
			}

			m_Endianess = Endianess_Big;
		}

		m_ReadPos += m_pFile->Read(&m_Magic, 4);

		if (m_Endianess == Endianess_Big)
		{
			m_Header.FileSize = PARSEDWORD_BIG(m_Header.FileSize);
			m_Magic = PARSEDWORD_BIG(m_Magic);
		}

		m_CurrentHunk.Name = m_Header.Magic;
		m_CurrentHunk.Size = m_Header.FileSize;
		PushForm();

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSFI::PushForm()
	{
		// FUN_00688160

		if (m_CurrentHunk.Name != TMAKEFOUR("FORM") &&
			m_CurrentHunk.Name != TMAKEFOUR("TSFL") &&
			m_CurrentHunk.Name != TMAKEFOUR("TSFB"))
		{
			return TTRB::ERROR_WRONG_MAGIC;
		}

		m_FileInfo[m_FileInfoCount].FileStartOffset = m_pFile->Tell() - 4;
		m_FileInfo[m_FileInfoCount].FileSize = m_CurrentHunk.Size;
		m_FileInfoCount++;

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSFI::PopForm()
	{
		// FUN_006881B0
		if (m_FileInfoCount < 1) return TTRB::ERROR_NO_FILEINFO_ON_STACK;
		
		m_FileInfoCount--;
		auto& fileInfo = m_FileInfo[m_FileInfoCount];

		m_CurrentHunk.Size = fileInfo.FileSize;
		m_ReadPos = m_pFile->Tell() - fileInfo.FileStartOffset;

		uint32_t alignedPos = TMath::AlignNumUp(m_CurrentHunk.Size);
		m_pFile->Seek(alignedPos - m_ReadPos, TFile::TSEEK_CUR);
		m_ReadPos = alignedPos;

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSFI::ReadHunk()
	{
		// FUN_00687fa0
		m_pFile->Read(&m_CurrentHunk, sizeof(Hunk));
		
		if (m_Endianess != Endianess_Little)
		{
			m_CurrentHunk.Name = PARSEDWORD_BIG(m_CurrentHunk.Name);
			m_CurrentHunk.Size = PARSEDWORD_BIG(m_CurrentHunk.Size);
		}

		m_ReadPos = 0;

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSFI::SkipHunk()
	{
		// FUN_006880e0
		uint32_t alignedSize = TMath::AlignNumUp(m_CurrentHunk.Size);
		m_pFile->Seek(alignedSize - m_ReadPos, TFile::TSEEK_CUR);
		m_ReadPos = alignedSize;

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSFI::ReadFORM(TTRB::SectionFORM* section)
	{
		// FUN_00688120
		if (m_CurrentHunk.Name != TMAKEFOUR("FORM"))
		{
			return TTRB::ERROR_WRONG_MAGIC;
		}

		m_ReadPos += m_pFile->Read(section, sizeof(TTRB::SectionFORM));

		return TTRB::ERROR_OK;
	}

	TTRB::ERROR TTSFI::ReadHunkData(void* dst)
	{
		if (m_CurrentHunk.Name == TMAKEFOUR("FORM"))
		{
			return TTRB::ERROR_FORM_MAGIC;
		}

		TASSERT(m_ReadPos == 0, "m_ReadPos should be zero");
		m_ReadPos += m_pFile->Read(dst, m_CurrentHunk.Size);
		m_ReadPos += TTSFI::ReadAlignmentPad();

		return TTRB::ERROR_OK;
	}

	void TTSFI::Close(bool free)
	{
		PopForm();

		if (m_pFile != TNULL && free)
		{
			m_pFile->Destroy();
		}

		m_pFile = TNULL;
		m_FileInfoCount = 0;
	}

	void TTSFI::ReadCompressed(void* buffer, uint32_t size)
	{
		TCompress::Header header;

		uint32_t headerStart = m_pFile->Tell();
		int8_t error = TCompress_Decompress::GetHeader(m_pFile, header);

		if (error == TCOMPRESS_ERROR_OK)
		{
			uint32_t headerSize = m_pFile->Tell() - headerStart;
			TCompress_Decompress::Decompress(m_pFile, &header, (char*)buffer, size);
			m_ReadPos += header.CompressedSize + headerSize;
		}
	}

	void TTSFI::LogUnknownSection()
	{
#ifdef TOSHI_DEBUG
		char charName[sizeof(m_CurrentHunk.Name) + 1] = {};
		*charName = m_CurrentHunk.Name;

		TOSHI_CORE_ERROR("Unknown TRB section: {0}", charName);
#endif
	}

	TTSFI::TTSFI() : m_FileInfo(), m_Header()
	{
		m_pFile = TNULL;
		m_FileInfoCount = 0;
		m_ReadPos = 0;
		// FUN_007EC9F0(m_FileInfo, 0, 0x100);
		m_Magic = 0;
		m_CurrentHunk.Name = 0;
		m_CurrentHunk.Size = 0;
		m_Endianess = Endianess_Little;
	}

	TTSFO::ERROR TTSFO::Create(const char* filepath, const char* magic, Endianess endianess)
	{
		m_pFile = TFile::Create(filepath, TFile::FileMode_CreateNew | TFile::FileMode_Write);

		if (m_pFile != TNULL)
		{
			TTSF::Hunk hunk{ TMAKEFOUR("TSFL"), 0 };
			uint32_t magicValue = TMAKEFOUR(magic);

			if (endianess == Endianess_Big)
			{
				hunk.Name = TMAKEFOUR("TSFB");
				hunk.Size = PARSEDWORD_BIG(hunk.Size);
			}

			Write(hunk);
			Write(magicValue);

			m_Endianess = endianess;
			m_Positions[m_PositionCount] = 0;
			m_PositionCount += 1;

			return TTRB::ERROR_OK;
		}
		else
		{
			return TTSFO::ERROR_FILE;
		}
	}

	void TTSFO::Close()
	{
		if (m_pFile != TNULL)
		{
			EndForm();
			m_pFile->Destroy();
			m_pFile = TNULL;
		}
	}

	size_t TTSFO::BeginForm(const char* name)
	{
		TASSERT(m_pFile != TNULL, "TTSFO is not created");

		TTSF::Hunk hunk{ TMAKEFOUR("FORM"), 0 };
		auto nameValue = TMAKEFOUR(name);

		if (m_Endianess == Endianess_Big)
		{
			hunk.Name = PARSEDWORD_BIG(hunk.Name);
			nameValue = PARSEDWORD_BIG(nameValue);
		}

		m_Positions[m_PositionCount++] = m_pFile->Tell();

		auto written1 = Write(hunk);
		auto written2 = Write(nameValue);

		return written1 + written2;
	}

	size_t TTSFO::EndForm()
	{
		TASSERT(m_pFile != TNULL, "TTSFO is not created");

		if (m_PositionCount > 0)
		{
			auto formPosition = m_Positions[--m_PositionCount];

			auto oldPos = m_pFile->Tell();
			auto formSize = (oldPos - formPosition) - 8;

			if (m_Endianess == Endianess_Big)
			{
				formSize = PARSEDWORD_BIG(formSize);
			}

			m_pFile->Seek(formPosition + 4, TFile::TSEEK_SET);
			Write(formSize);
			m_pFile->Seek(oldPos, TFile::TSEEK_SET);

			return formSize;
		}

		return 0;
	}

	bool TTSFO::OpenHunk(HunkMark* hunkMark, const char* hunkName)
	{
		TASSERT(hunkMark != TNULL, "HunkMark is TNULL");
		hunkMark->Name = TMAKEFOUR(hunkName);
		hunkMark->Pos = m_pFile->Tell();

		WriteHunk(hunkMark->Name, TNULL, 0);
		return true;
	}

	bool TTSFO::CloseHunk(HunkMark* hunkMark)
	{
		TASSERT(hunkMark != TNULL, "HunkMark is TNULL");
		auto oldPos = m_pFile->Tell();

		m_pFile->Seek(hunkMark->Pos, TFile::TSEEK_SET);
		WriteHunk(hunkMark->Name, TNULL, (oldPos - hunkMark->Pos) - sizeof(TTSF::Hunk));
		m_pFile->Seek(oldPos, TFile::TSEEK_SET);
		TTSFO::WriteAlignmentPad();

		return true;
	}

	size_t TTSFO::WriteHunk(uint32_t hunkName, void* buffer, size_t bufferSize)
	{
		TTSF::Hunk hunk{ hunkName, bufferSize };

		// Convert endianess
		if (m_Endianess == Endianess_Big)
		{
			hunk.Name = PARSEDWORD_BIG(hunk.Name);
			hunk.Size = PARSEDWORD_BIG(hunk.Size);
		}

		// Write hunk
		size_t writtenSize = Write(hunk);

		// Write buffer
		if (buffer != TNULL && bufferSize > 0)
		{
			writtenSize += m_pFile->Write(buffer, bufferSize);
		}

		// Return num of written bytes
		return writtenSize + TTSFO::WriteAlignmentPad();
	}
}