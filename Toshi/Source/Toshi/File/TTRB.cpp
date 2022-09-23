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
		uint8_t error = ttsf.ReadFile(file);

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
					//TTODO("SECC section");
					SecInfo* pSects = reinterpret_cast<SecInfo*>(m_pHeader + 1);

					for (size_t i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						if (pSects[i].m_pData != TNULL)
						{
							ttsf.DecompressSection(pSects[i].m_pData, pSects[i].m_Size);
						}
					}

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

							SecInfo* pSects = reinterpret_cast<SecInfo*>(m_pHeader + 1);
							for (uint32_t i = 0; i < relocReadCount; i++)
							{
								auto& relcEntry = relcEntries[i];
								auto& hdrx1 = pSects[relcEntry.HDRX1];
								auto& hdrx2 = hdrx1;

								if (m_pHeader->m_ui32Version >= TMAKEVERSION(1, 0))
								{
									hdrx2 = pSects[relcEntry.HDRX2];
								}

								// this won't work in x64 because pointers in TRB files are always 4 bytes
								// need some workaround to support x64 again
								uintptr_t* ptr = reinterpret_cast<uintptr_t*>((uintptr_t)hdrx1.m_pData + relcEntry.Offset);
								*ptr += (uintptr_t)hdrx2.m_pData;
							}

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
					SecInfo* pSect = reinterpret_cast<SecInfo*>(m_pHeader + 1);
					for (uint32_t i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						ttsf.ReadBytes(pSect->m_pData, pSect->m_Size);
						pSect++;
					}

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("HDRX"))
				{
					m_pHeader = static_cast<Header*>(tmalloc(sectionSize));
					ttsf.ReadSectionData(m_pHeader);
					
					SecInfo* pSect = reinterpret_cast<SecInfo*>(m_pHeader + 1);
					for (uint32_t i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						pSect->m_Unk1 = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
						pSect->m_pData = tmalloc(pSect->m_Size);
						pSect++;
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
