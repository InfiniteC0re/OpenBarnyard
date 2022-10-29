#include "Toshi/File/TFile.h"
#include <Toshi/Core/Core.h>

namespace Toshi
{
	class TLogFile
	{
		const char* cTypeStrings[3]
		{
			"Info",
			"Warning",
			"Error"
		};

		TFile* m_pFile; // 0x0
		int m_unk; // 0x2C
		int* m_unkCounts; // 0x30

		~TLogFile()
		{
			TASSERT(TNULL == m_pFile, "TFile must be NULL");
			Close();
		}

		void Close();
		void RecalcLevel();
		void Print(const char*, ...);
	};
}