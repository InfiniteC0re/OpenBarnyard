#pragma once
#include "SECT.h"

namespace TLib
{
	namespace TRBF
	{
		class RELC : public Hunk
		{
		public:
			RELC() = default;

			void Write(Toshi::TTSFO& ttsfo, SECT& sect)
			{
				uint32_t ptrCount = 0;
				for (auto section : sect)
				{
					ptrCount += section->GetPointerCount();
				}

				ttsfo.Write(ptrCount);

				for (size_t i = 0; i < sect.GetSectionCount(); i++)
				{
					auto section = sect.GetSection(i);

					for (auto& ptr : *section)
					{
						Toshi::TTRB::RELCEntry entry = { };
						entry.HDRX1 = (short)i;
						entry.HDRX2 = (short)i;
						entry.Offset = ptr.Offset;
						ttsfo.Write(entry);
					}
				}
			}

			void Read(Toshi::TTSFI& ttsfi, SECT& sect)
			{
				uint32_t ptrCount = 0;
				ttsfi.Read(&ptrCount);

				Toshi::TTRB::RELCEntry entry;
				for (uint32_t i = 0; i < ptrCount; i++)
				{
					ttsfi.Read(&entry);
					auto stack = sect.GetSection(entry.HDRX1);
					uint32_t dataPtr = *(uint32_t*)(&stack->GetBuffer()[entry.Offset]);
					stack->AddRelocationPtr(entry.Offset, dataPtr);
				}

				for (auto stack : sect)
				{
					stack->Link();
				}
			}
		};
	}
}