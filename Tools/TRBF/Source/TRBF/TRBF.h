#pragma once
#include "TRBF/Hunks/HDRX.h"
#include "TRBF/Hunks/SECT.h"
#include "TRBF/Hunks/RELC.h"

class TRBF
{
public:
	static constexpr TVersion VERSION = { TMAKEVERSION(1, 1) };

public:
	TRBF() : m_HDRX(VERSION)
	{
		
	}

	void WriteToFile(const std::string& filepath, Toshi::TTSF::Endianess endianess = Toshi::TTSF::Endianess_Little)
	{
		Toshi::TTSFO ttsfo;
		Toshi::TTSFO::HunkMark mark;
		ttsfo.Create(filepath.c_str(), "TRBF", endianess);
		
		// HDRX
		ttsfo.OpenHunk(&mark, "HDRX");
		m_HDRX.SetSectionCount(m_SECT.GetSectionCount());
		m_HDRX.Write(ttsfo, m_SECT);
		ttsfo.CloseHunk(&mark);
		
		// SECT
		ttsfo.OpenHunk(&mark, "SECT");
		m_SECT.Write(ttsfo);
		ttsfo.CloseHunk(&mark);

		// RELC
		ttsfo.OpenHunk(&mark, "RELC");
		m_RELC.Write(ttsfo, m_SECT);
		ttsfo.CloseHunk(&mark);

		ttsfo.Close();
	}

	SECT* GetSECT()
	{
		return &m_SECT;
	}

private:
	HDRX m_HDRX;
	SECT m_SECT;
	RELC m_RELC;
};