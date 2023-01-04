#pragma once
#include "Hunk.h"

#include <Toshi/File/TTRB.h>
#include <Toshi/File/TTSF.h>

class HDRX : public Hunk
{
public:
	HDRX(TVersion version)
	{
		m_Header.m_ui32Version = version;
		m_Header.m_i32SectionCount = 0;
	}

	void SetVersion(TVersion version)
	{
		m_Header.m_ui32Version = version;
	}

	void SetSectionCount(int32_t count)
	{
		m_Header.m_i32SectionCount = count;
	}

	void Write(Toshi::TTSFO& ttsfo)
	{
		ttsfo.Write(m_Header);
	}

private:
	Toshi::TTRB::Header m_Header;
};