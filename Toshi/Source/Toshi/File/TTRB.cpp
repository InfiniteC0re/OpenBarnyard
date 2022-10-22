#include "ToshiPCH.h"
#include "TTRB.h"
#include "TTSF.h"

namespace Toshi
{
	TTRB::t_MemoryFuncAlloc TTRB::s_cbDefAllocator = [](AllocType alloctype, size_t size, short unk1, size_t unk2, void* userData)
	{
		return tmalloc(size);
	};

	TTRB::t_MemoryFuncDealloc TTRB::s_cbDefDeallocator = [](AllocType alloctype, void* ptr, short unk1, size_t unk2, void* userData)
	{
		tfree(ptr);
	};

	void* TTRB::s_pDefAllocatorUserData = nullptr;

	TTRB::TTRB()
	{
		m_pHeader = TNULL;
		m_SYMB = TNULL;

		SetMemoryFunctions(s_cbDefAllocator, s_cbDefDeallocator, s_pDefAllocatorUserData);
	}

	TTRB::ERROR TTRB::Load(const char* path)
	{
		// FUN_006868e0
		TFile* pFile = TFile::Create(path, TFile::OpenFlags_Read);
		return Load(pFile);
	}

	TTRB::ERROR TTRB::Load(TFile* file)
	{
		// FUN_00686920
		TTSFI ttsf;
		ERROR error = ttsf.Open(file);

		if (error == ERROR_OK)
		{
			if (ttsf.m_TRBF == TMAKEFOUR("TRBF"))
			{
				if (ProcessForm(ttsf))
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

		return error;
	}

	bool TTRB::ProcessForm(TTSFI& ttsf)
	{
		// FUN_00686f10
		static constexpr uint32_t s_RELCEntriesLimit = 0x200;
		RELCEntry relcEntries[s_RELCEntriesLimit];

		int32_t fileSize = ttsf.m_CurrentSection.Size - 4;
		int32_t leftSize = fileSize;
		ttsf.PushForm();

		do
		{
			uint32_t sectionName = 0;
			uint32_t sectionSize = 0;

			while (true)
			{
				if (fileSize < 1)
				{
					ttsf.PopForm();
					// FUN_007ebfbf
					return true;
				}

				uint8_t readResult = ttsf.ReadHunk();
				if (readResult != ERROR_OK) return false;

				sectionName = ttsf.m_CurrentSection.Name;
				sectionSize = ttsf.m_CurrentSection.Size;
				leftSize -= TMath::AlignNumUp(sectionSize) + 8;

				if (TMAKEFOUR("HEAD") < sectionName) break;

				if (sectionName == TMAKEFOUR("HEAD"))
				{
					int numsections = (sectionSize - 4) / 0xC;

					m_pHeader = static_cast<Header*>(m_MemAllocator(AllocType_Unk0, sizeof(Header) + sizeof(SecInfo) * numsections, 0, 0, m_MemUserData));
					m_pHeader->m_ui32Version = 0;

					ttsf.ReadRaw(&m_pHeader->m_i32SectionCount, sizeof(m_pHeader->m_i32SectionCount));

					TASSERT(m_pHeader->m_i32SectionCount == numsections, "HEAD section has wrong num of sections");

					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						SecInfo* pSect = GetSectionInfo(i);
						
						ttsf.ReadRaw(pSect, 0xC);
						pSect->m_Data = m_MemAllocator(AllocType_Unk1, pSect->m_Size, 0, 0, m_MemUserData);
						pSect->m_Unk1 = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
						pSect->m_Unk2 = 0;
					}

					ttsf.SkipHunk();
				}
				else if (sectionName == TMAKEFOUR("SYMB"))
				{
					m_SYMB = static_cast<SYMB*>(m_MemAllocator(AllocType_Unk2, ttsf.m_CurrentSection.Size, 0, 0, m_MemUserData));
					ttsf.ReadHunkData(m_SYMB);
				}
				else if (sectionName == TMAKEFOUR("SECC"))
				{
					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						auto* secInfo = GetSectionInfo(i);

						if (secInfo->m_Data != TNULL)
						{
							ttsf.ReadCompressed(secInfo->m_Data, secInfo->m_Size);
						}
					}

					ttsf.SkipHunk();
				}
				else if (sectionName == TMAKEFOUR("RELC"))
				{
					uint32_t relocCount = 0;
					uint32_t curReloc = 0;
					uint32_t readedRelocs = 0;

					ttsf.ReadRaw(&relocCount, sizeof(relocCount));

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
							ttsf.ReadRaw(relcEntries, relocReadCount << 3);
							curReloc = readedRelocs + relocReadCount;

							auto& header = *m_pHeader;
							for (uint32_t i = 0; i < relocReadCount; i++)
							{
								auto& relcEntry = relcEntries[i];
								auto hdrx1 = GetSectionInfo(relcEntry.HDRX1);
								auto hdrx2 = hdrx1;

								if (m_pHeader->m_ui32Version >= TMAKEVERSION(1, 0))
								{
									hdrx2 = GetSectionInfo(relcEntry.HDRX2);
								}

								// this won't work in x64 because pointers in TRB files are always 4 bytes
								// need some workaround to support x64 again
								uintptr_t* ptr = reinterpret_cast<uintptr_t*>((uintptr_t)hdrx1->m_Data + relcEntry.Offset);
								*ptr += (uintptr_t)hdrx2->m_Data;
							}

							readedRelocs += relocReadCount;
						} while (curReloc < relocCount);
					}

					ttsf.SkipHunk();
				}
				else
				{
					// Unknown section
					ttsf.SkipHunk();

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
						SecInfo* pSect = GetSectionInfo(i);
						ttsf.ReadRaw(pSect->m_Data, pSect->m_Size);
					}

					ttsf.SkipHunk();
				}
				else if (sectionName == TMAKEFOUR("HDRX"))
				{
					m_pHeader = static_cast<Header*>(m_MemAllocator(AllocType_Unk0, sectionSize, 0, 0, m_MemUserData));
					ttsf.ReadHunkData(m_pHeader);
					
					for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
					{
						SecInfo* pSect = GetSectionInfo(i);
						pSect->m_Unk1 = (pSect->m_Unk1 == 0) ? 16 : pSect->m_Unk1;
						pSect->m_Data = m_MemAllocator(AllocType_Unk1, pSect->m_Size, pSect->m_Unk1, pSect->m_Unk2, m_MemUserData);
					}
				}
				else
				{
					// Unknown section
					ttsf.SkipHunk();

#ifdef TOSHI_DEBUG
					ttsf.LogUnknownSection();
#endif
				}

				fileSize = leftSize;
			}
		} while (true);

		SectionFORM form;
		ttsf.ReadFORM(&form);

		bool result = ProcessForm(ttsf);
		fileSize = leftSize;

		return result;
	}

	void* TTRB::GetSymbolAddress(const char* symbName)
	{
		// FUN_00686d30
		auto index = GetSymbolIndex(symbName);

		if (m_SYMB != TNULL && index != -1 && index < m_SYMB->m_i32SymbCount)
		{
			auto entry = GetSymbol(index);
			return static_cast<char*>(GetSection(entry->HDRX)) + entry->DataOffset;
		}

		return TNULL;
	}

	int TTRB::GetSymbolIndex(const char* symbName)
	{
		// 00686c30
		if (m_SYMB != TNULL)
		{
			short hash = HashString(symbName);

			for (int i = 0; i < m_SYMB->m_i32SymbCount; i++)
			{
				auto symbol = GetSymbol(i);

				if (symbol->NameHash == hash)
				{
					if (Toshi2::T2String8::CompareStrings(symbName, GetSymbolName(symbol), -1) == 0)
					{
						return i;
					}
				}
			}
		}

		return -1;
	}

	void TTRB::Close()
	{
		// FUN_006869d0
		if (m_pHeader != TNULL)
		{
			for (int i = 0; i < m_pHeader->m_i32SectionCount; i++)
			{
				auto sec = GetSectionInfo(i);

				if (sec->m_Data != TNULL)
				{
					m_MemDeallocator(AllocType_Unk1, sec->m_Data, sec->m_Unk1, sec->m_Unk2, m_MemUserData);
				}
			}

			m_MemDeallocator(AllocType_Unk0, m_pHeader, 0, 0, m_MemUserData);
			m_pHeader = TNULL;
		}

		DeleteSymbolTable();
	}
}
