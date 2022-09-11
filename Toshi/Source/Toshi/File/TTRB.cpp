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
					fileSize = leftSize;
				}
				else if (sectionName == TMAKEFOUR("SYMB"))
				{
					TTODO("SYMB section");
					
					char* SYMBData = (char*)tmalloc(ttsf.m_CurrentSection.Size);
					ttsf.ReadSectionData(SYMBData);
					tfree(SYMBData);

					fileSize = leftSize;
				}
				else if (sectionName == TMAKEFOUR("SECC"))
				{
					TTODO("SECC section");

					ttsf.SkipSection();
					fileSize = leftSize;
				}
				else if (sectionName == TMAKEFOUR("RELC"))
				{
					TTODO("RELC section");

					ttsf.SkipSection();
					fileSize = leftSize;
				}
				else
				{
					// Unknown section
					ttsf.SkipSection();
					fileSize = leftSize;

#ifdef TOSHI_DEBUG
					ttsf.LogUnknownSection();
#endif
				}
			}

			if (sectionName != TMAKEFOUR("FORM"))
			{
				if (sectionName == TMAKEFOUR("SECT"))
				{
					TTODO("SECT section");

					ttsf.SkipSection();
					fileSize = leftSize;
				}
				else if (sectionName == TMAKEFOUR("HDRX"))
				{
					TTODO("HDRX section");

					ttsf.SkipSection();
					fileSize = leftSize;
				}
				else
				{
					// Unknown section
					ttsf.SkipSection();
					fileSize = leftSize;

#ifdef TOSHI_DEBUG
					ttsf.LogUnknownSection();
#endif
				}
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
