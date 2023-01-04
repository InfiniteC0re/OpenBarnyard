#pragma once
#include "SECT.h"

namespace TTool
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
		};
	}
}