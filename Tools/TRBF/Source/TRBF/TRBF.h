#pragma once
#include "TRBF/Hunks/HDRX.h"

#include <Toshi/File/TTRB.h>
#include <Toshi/File/TTSF.h>

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
		ttsfo.Create(filepath.c_str(), "TRBF", endianess);
		
		Toshi::TTSFO::HunkMark hdrxMark;
		ttsfo.OpenHunk(&hdrxMark, "HDRX");
		m_HDRX.Write(ttsfo);
		TTODO("Write info about sections");
		ttsfo.CloseHunk(&hdrxMark);

		ttsfo.Close();
	}

private:
	HDRX m_HDRX;

};