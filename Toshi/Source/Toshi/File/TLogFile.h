#pragma once
#include "Toshi/File/TFile.h"

namespace Toshi
{
	class TLogFile : public TSingleton<TLogFile>
	{
		static inline const char* cTypeStrings[3]
		{
			"Info",
			"Warning",
			"Error"
		};

		static constexpr uint32_t cLevelMax = 30;

		TFile* m_pFile;                 // 0x0
		int m_iTotalLogCount;           // 0x4
		bool m_bIsSimpleMode;           // 0x8
		bool m_bAllowIndentation;       // 0x9
		char m_LevelString[cLevelMax];  // 0xA
		uint32_t m_curLevel;                 // 0x2C
		int m_typeCounts[3];              // 0x30
		int* m_unk2;                    // 0x34
		int m_unk3;                     // 0x38

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

		int Create(const char* fileName, const char* str2, bool writeExisting);
		void Close();
		void RecalcLevel();
		void Print(const char* format, ...);

		void Log(TYPE type, const char* str1, const char* str2, const char* format, ...);

		void Down()
		{
			m_curLevel--;
			RecalcLevel();
		}

		void Up()
		{
			m_curLevel++;
			RecalcLevel();
		}

		void AllowIndentation(bool allow) { m_bAllowIndentation = allow; }
		void SetSimpleMode(bool enable) { m_bIsSimpleMode = enable; }

		int GetTypeCount(TYPE type) const { return m_typeCounts[type]; }
		int GetTotalLogCount() const { return m_iTotalLogCount; }
		const char* GetLevelString() const { return m_LevelString; }
	};
}