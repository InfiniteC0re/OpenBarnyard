#pragma once
#include "SECT.h"

class RELC : public Hunk
{
public:
	RELC()
	{

	}

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
				entry.HDRX1 = i;
				entry.HDRX2 = i;
				entry.Offset = ptr.Offset;
				ttsfo.Write(entry);
			}
		}
	}
};