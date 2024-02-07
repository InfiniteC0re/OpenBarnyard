#pragma once
#include "File/TFile.h"

namespace Toshi
{
	class TLogFile : public TSingleton<TLogFile>
	{
	private:
		static constexpr uint32_t cLevelMax = 30;

	public:
		enum Type
		{
			Type_Info,
			Type_Warning,
			Type_Error,
			Type_NUMOF,
		};

		static constexpr const char* cTypeStrings[]
		{
			"Info",
			"Warning",
			"Error"
		};

		TSTATICASSERT(Type_NUMOF == TARRAYSIZE(cTypeStrings));

		enum class Error
		{
			OK,
			OpenFile,
			AlreadyCreated
		};

	public:
		TLogFile();

		~TLogFile()
		{
			TASSERT(TNULL == m_pFile, "TFile must be NULL");
			Close();
		}

		Error Create(const char* fileName, const char* str2, TBOOL writeExisting);
		void Close();
		void RecalcLevel();
		void Print(const char* format, ...);

		void Log(Type type, const char* str1, const char* str2, const char* format, ...);

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

		void AllowIndentation(TBOOL allow)
		{
			m_bAllowIndentation = allow; 
		}
		
		void SetSimpleMode(TBOOL enable)
		{
			m_bIsSimpleMode = enable;
		}

		int GetTypeCount(Type type) const
		{
			return m_typeCounts[type];
		}

		int GetTotalLogCount() const
		{
			return m_iTotalLogCount;
		}

		const char* GetLevelString() const
		{
			return m_LevelString;
		}

	private:
		TFile* m_pFile;                 // 0x00
		int m_iTotalLogCount;           // 0x04
		TBOOL m_bIsSimpleMode;           // 0x08
		TBOOL m_bAllowIndentation;       // 0x09
		char m_LevelString[cLevelMax];  // 0x0A
		uint32_t m_curLevel;            // 0x2C
		int m_typeCounts[Type_NUMOF];   // 0x30
		int* m_unk2;                    // 0x34
		int m_unk3;                     // 0x38
	};
}