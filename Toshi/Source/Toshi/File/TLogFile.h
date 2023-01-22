#pragma once
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TLogFile : public TSingleton<TLogFile>
	{
		const char* cTypeStrings[3]
		{
			"Info",
			"Warning",
			"Error"
		};

		uint32_t cLevelMax = 30;

		TFile* m_pFile;           // 0x0
		int m_iTotalLogCount;     // 0x4
		bool m_bIsSimpleMode;     // 0x8
		bool m_bAllowIndentation; // 0x9
		char* m_LevelString;      // 0xA
		int m_unk;                // 0x2C
		int* m_typeCounts;        // 0x30
		int* m_unk2;              // 0x34
		int m_unk3;               // 0x38

	public:

		enum TYPE
		{
			TYPE_Info,
			TYPE_Warning,
			TYPE_Error,
		};

		TLogFile();

		~TLogFile()
		{
			TASSERT(TNULL == m_pFile, "TFile must be NULL");
			Close();
		}

		void Close();
		void RecalcLevel();
		void Print(const char*, ...);

		void Log(TYPE type, const char* str1, const char* str2, const char* format, ...);

		int GetTypeCount(TYPE type) const { return m_typeCounts[type]; }
		int GetTotalLogCount() const { return m_iTotalLogCount; }
		const char* GetLevelString() const { return m_LevelString; }
	};
}