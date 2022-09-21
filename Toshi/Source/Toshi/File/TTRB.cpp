#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

namespace Toshi
{
	bool TTRB::LoadTrb(const char* fn)
	{
		TTSF ttsf = TTSF();

		TCString str = TCString(fn);
		auto fs = Toshi::TFileSystem::CreateNative(str);
		
		Toshi::TFile* file = fs->CreateFile(str, 1);
		int error = ttsf.ReadFile(file);

		if (error == TTRB_ERROR::ERROR_OK)
		{
			ReadTrb(ttsf);
		}

		return false;
	}

	bool TTRB::ReadTrb(TTSF& ttsf)
	{
		static constexpr uint32_t RELCEntriesLimit = 0x200;
		RELCEntry relcEntries[RELCEntriesLimit];

		int32_t fileSize = ttsf.m_CurrentSection.Size - 4;
		int32_t leftSize = fileSize;
		ttsf.PushFileInfo();

		do
		{
			uint32_t sectionName = 0;
			uint32_t sectionSize = 0;

			while (true)
			{
				if (fileSize < 1)
				{
					ttsf.PopFileInfo();
					// FUN_007ebfbf
					return true;
				}

				uint8_t readResult = ttsf.ReadSectionHeader();
				if (readResult != ERROR_OK) return false;

				//TASSERT(readResult == ERROR_OK, "Error in ReadSectionHeader");
				
				sectionName = ttsf.m_CurrentSection.Name;
				sectionSize = ttsf.m_CurrentSection.Size;
				leftSize -= TMath::AlignNumUp(sectionSize) + 8;

				if (TMAKEFOUR("HEAD") < sectionName) break;

				if (sectionName == TMAKEFOUR("HEAD"))
				{
					TTODO("HEAD section");

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("SYMB"))
				{
					TTODO("SYMB section");
					
					char* SYMBData = (char*)tmalloc(ttsf.m_CurrentSection.Size);
					ttsf.ReadSectionData(SYMBData);
					tfree(SYMBData);
				}
				else if (sectionName == TMAKEFOUR("SECC"))
				{
					TTODO("SECC section");

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("RELC"))
				{
					TTODO("RELC section");

					uint32_t relocCount = 0;
					uint32_t curReloc = 0;
					uint32_t readedRelocs = 0;

					ttsf.ReadBytes(&relocCount, sizeof(relocCount));

					if (relocCount < 1)
					{
						relocCount = 0;
					}
					else
					{
						do
						{
							uint32_t relocReadCount = relocCount - readedRelocs;

							// limit count of RELCs to read
							relocReadCount = TMath::Min(relocReadCount, RELCEntriesLimit);
							ttsf.ReadBytes(relcEntries, relocReadCount << 3);
							curReloc = readedRelocs + relocReadCount;

							// TODO..... 0x0068704f

						} while (curReloc < relocCount);
					}

					ttsf.SkipSection();
				}
				else
				{
					// Unknown section
					ttsf.SkipSection();

#ifdef TOSHI_DEBUG
					ttsf.LogUnknownSection();
#endif
				}

				fileSize = leftSize;
			}

			if (sectionName != TMAKEFOUR("FORM"))
			{
				if (sectionName == TMAKEFOUR("SECT"))
				{
					SecInfo* section = reinterpret_cast<SecInfo*>(m_pHeader + 1);
					for (uint32_t i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						ttsf.ReadBytes(section->m_pData, section->m_Size);
						section += 1;
					}

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("HDRX"))
				{
					m_pHeader = (Header*)tmalloc(sectionSize);
					ttsf.ReadSectionData(m_pHeader);

					SecInfo* section = (SecInfo*)(m_pHeader + 1);
					for (uint32_t i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						section->m_Unk1 = (section->m_Unk1 == 0) ? 16 : section->m_Unk1;
						section->m_pData = tmalloc(section->m_Size);
						section += 1;
					}
				}
				else
				{
					// Unknown section
					ttsf.SkipSection();

#ifdef TOSHI_DEBUG
					ttsf.LogUnknownSection();
#endif
				}

				fileSize = leftSize;
			}
		} while (true);

		SectionFORM form;
		ttsf.ReadFORM(&form);
		bool result = ReadTrb(ttsf);
		fileSize = leftSize;

		if (result == false)
		{
			return false;
		}
	}
}
