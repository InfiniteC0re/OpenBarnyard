#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

namespace Toshi
{
	TTRB::ERROR TTRB::Open(const char* path)
	{
		// FUN_006868e0
		TFile* pFile = TFile::Create(path, TFile::OpenFlags_Read);
		return Open(pFile);
	}

	TTRB::ERROR TTRB::Open(TFile* file)
	{
		// FUN_00686920
		TTSF ttsf;
		ERROR error = ttsf.ReadFile(file);

		if (error == ERROR_OK)
		{
			if (ttsf.m_TRBF == TMAKEFOUR("TRBF"))
			{
				if (Parse(ttsf))
				{
					error = ERROR_OK;
				}
				else
				{
					error = ERROR_PARSE_ERROR;
				}
			}
			else
			{
				error = ERROR_NOT_TRBF;
			}
		}
		else
		{
			error = ERROR_NO_HEADER;
		}

		ttsf.Destroy();
		return error;
	}

	bool TTRB::Parse(TTSF& ttsf)
	{
		// FUN_00686f10
		static constexpr uint32_t s_RELCEntriesLimit = 0x200;
		RELCEntry relcEntries[s_RELCEntriesLimit];

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

				sectionName = ttsf.m_CurrentSection.Name;
				sectionSize = ttsf.m_CurrentSection.Size;
				leftSize -= TMath::AlignNumUp(sectionSize) + 8;

				if (TMAKEFOUR("HEAD") < sectionName) break;

				if (sectionName == TMAKEFOUR("HEAD"))
				{
					int numsections = (sectionSize - 4) / 0xC;
					m_pHeader = static_cast<Header*>(tmalloc(sizeof(Header) + sizeof(SecInfo) * numsections));
					m_pHeader->m_ui32Version = 0;

					ttsf.ReadBytes(&m_pHeader->m_i32SectionCount, sizeof(m_pHeader->m_i32SectionCount));

					TASSERT(m_pHeader->m_i32SectionCount == numsections, "HEAD section has wrong num of sections");

					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						SecInfo* pSect = m_pHeader->GetSecInfo(i);
						
						ttsf.ReadBytes(pSect, 0xC);
						pSect->m_pData = tmalloc(pSect->m_Size);
						pSect->m_Unk1 = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
						pSect->m_Unk2 = 0;
					}

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("SYMB"))
				{
					m_SYMB = static_cast<SYMB*>(tmalloc(ttsf.m_CurrentSection.Size));
					ttsf.ReadSectionData(m_SYMB);
				}
				else if (sectionName == TMAKEFOUR("SECC"))
				{
					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						auto* secInfo = m_pHeader->GetSecInfo(i);

						if (secInfo->m_pData != TNULL)
						{
							ttsf.DecompressSection(secInfo->m_pData, secInfo->m_Size);
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
							relocReadCount = TMath::Min(relocReadCount, s_RELCEntriesLimit);
							ttsf.ReadBytes(relcEntries, relocReadCount << 3);
							curReloc = readedRelocs + relocReadCount;

							auto& header = *m_pHeader;
							for (uint32_t i = 0; i < relocReadCount; i++)
							{
								auto& relcEntry = relcEntries[i];
								auto& hdrx1 = *m_pHeader->GetSecInfo(relcEntry.HDRX1);
								auto& hdrx2 = hdrx1;

								if (m_pHeader->m_ui32Version >= TMAKEVERSION(1, 0))
								{
									hdrx2 = *m_pHeader->GetSecInfo(relcEntry.HDRX2);
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
					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						SecInfo* pSect = m_pHeader->GetSecInfo(i);
						ttsf.ReadBytes(pSect->m_pData, pSect->m_Size);
					}

					ttsf.SkipSection();
				}
				else if (sectionName == TMAKEFOUR("HDRX"))
				{
					m_pHeader = static_cast<Header*>(tmalloc(sectionSize));
					ttsf.ReadSectionData(m_pHeader);
					
					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						SecInfo* pSect = m_pHeader->GetSecInfo(i);
						pSect->m_Unk1 = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
						pSect->m_pData = tmalloc(pSect->m_Size);
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

		bool result = Parse(ttsf);
		fileSize = leftSize;

		return result;
	}

	void* TTRB::FindSymb(const char* symbName)
	{
		// FUN_00686d30
		auto index = FindSymbIndex(symbName);

		if (m_SYMB != TNULL && index != -1 && index < m_SYMB->m_i32SymbCount)
		{
			auto& entry = (*m_SYMB)[index];
			return static_cast<char*>(GetSection(entry.HDRX)) + entry.Data_Offset;
		}

		return TNULL;
	}

	int TTRB::FindSymbIndex(const char* symbName)
	{
		// FUN_00686c30
		if (m_SYMB == TNULL) return -1;
		if (m_SYMB->m_i32SymbCount < 1) return -1;

		const char* iterator = symbName;
		short type_hash = 0;

		while (*iterator != '\0')
		{
			type_hash = ((type_hash << 5) - type_hash) + (short)*iterator++;
		}

		auto& symb = *m_SYMB;
		for (int i = 0; i < m_SYMB->m_i32SymbCount; i++)
		{
			if (symb[i].Type_Hash == type_hash)
			{
				const char* currentTypeName = symb.Names() + symb[i].Type_Offset;
				
				if (Toshi2::T2String8::CompareStrings(symbName, currentTypeName, -1) == 0)
				{
					return i;
				}
			}
		}

		return -1;
	}

	void TTRB::Destroy()
	{
		// FUN_006869d0
		if (m_pHeader != TNULL)
		{
			for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
			{
				DestroySection(i);
			}

			tfree(m_pHeader);
			m_pHeader = TNULL;
		}

		if (m_SYMB != TNULL)
		{
			tfree(m_SYMB);
			m_SYMB = TNULL;
		}
	}

	void TTRB::DestroySection(int index)
	{
		// FUN_00686a40
		TASSERT(index >= 0, "Index cannot be negative");
		TASSERT(index < m_pHeader->m_i32SectionCount, "Index is out of bounds");

		SecInfo* pSecInfo = m_pHeader->GetSecInfo(index);
		TASSERT(pSecInfo != TNULL, "pSecInfo is TNULL");

		if (pSecInfo->m_pData != TNULL)
		{
			tfree(pSecInfo->m_pData);
			pSecInfo->m_pData = TNULL;
		}
	}
}
