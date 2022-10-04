#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

namespace Toshi
{
	bool TTRB::LoadTrb(const char* fn)
	{
		TTSF ttsf;

		TCString str(fn);
		auto fs = Toshi::TFileSystem::CreateNative(str);
		
		Toshi::TFile* file = fs->CreateFile(str, 1);
		uint8_t error = ttsf.ReadFile(file);

		bool readResult = false;
		if (error == TTRB_ERROR::ERROR_OK)
		{
			if (ttsf.m_TRBF == TMAKEFOUR("TRBF"))
			{
				ReadTrb(ttsf);
				readResult = true;
			}
		}

		ttsf.Destroy();
		return readResult;
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

					int numsections = (sectionSize - 4) / 0xC;
					m_pHeader = static_cast<Header*>(tmalloc(sizeof(Header) + sizeof(SecInfo) * numsections));
					m_pHeader->m_ui32Version = 0;

					ttsf.ReadBytes(&m_pHeader->m_i32SectionCount, sizeof(m_pHeader->m_i32SectionCount));

					// TASSERT(m_pHeader->m_ui32Version == TMAKEVERSION(0, 0), "HEAD section cannot have a version");
					TASSERT(m_pHeader->m_i32SectionCount == numsections, "HEAD section has wrong num of sections");

					SecInfo* pCur = reinterpret_cast<SecInfo*>(m_pHeader + 1);
					for (size_t i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						ttsf.ReadBytes(pCur, 0xC);
						pCur->m_pData = tmalloc(pCur->m_Size);
						pCur->m_Unk1 = (pCur->m_Unk1 == 0) ? 16 : pCur->m_Unk1;
						pCur->m_Unk2 = 0;
						pCur++;
					}

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("SYMB"))
				{
					TTODO("SYMB section");
					
					m_SYMB = static_cast<SYMB*>(tmalloc(ttsf.m_CurrentSection.Size));
					ttsf.ReadSectionData(m_SYMB);
				}
				else if (sectionName == TMAKEFOUR("SECC"))
				{
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

							readedRelocs += relocReadCount;
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

	char* TTRB::GetSymb(const char* symbName)
	{
		int index = GetSymbFromSect(symbName);

		SYMBEntry* pSymbEntries = reinterpret_cast<SYMBEntry*>(m_SYMB + 1);
		SecInfo* pSects = reinterpret_cast<SecInfo*>(m_pHeader + 1);

		if (index != -1 && m_SYMB != TNULL && index < m_SYMB->m_i32SymbCount && &pSymbEntries[index] != TNULL)
		{
			return (char*)pSects[pSymbEntries[index].HDRX].m_pData + pSymbEntries[index].Data_Offset;
		}

		return TNULL;
	}

	int TTRB::GetSymbFromSect(const char* symbName)
	{
		if (m_SYMB == TNULL) return -1;
		if (m_SYMB->m_i32SymbCount < 1) return -1;

		const char* iterator = symbName;
		short type_hash = 0;

		while (*iterator != '\0')
		{
			type_hash = ((type_hash << 5) - type_hash) + (short)*iterator++;
		}

		SYMBEntry* pSymbEntries = reinterpret_cast<SYMBEntry*>(m_SYMB + 1);
		
		for (size_t i = 0; i < m_SYMB->m_i32SymbCount; i++)
		{
			if (pSymbEntries[i].Type_Hash == type_hash)
			{
				char* currentTypeName = (char*)&pSymbEntries[m_SYMB->m_i32SymbCount] + pSymbEntries[i].Type_Offset;
				if (Toshi2::T2String8::CompareStrings(symbName, currentTypeName, -1) == 0)
				{
					return i;
				}
			}
		}

		return -1;
	}
}
