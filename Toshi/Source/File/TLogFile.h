#pragma once

namespace Toshi {
	
	class TFile;

	class TLogFile
	{
	private:
		static constexpr TUINT32 cLevelMax = 30;

	public:
		enum Type
		{
			Type_Info,
			Type_Warning,
			Type_Error,
			Type_NUMOF,
		};

		enum class Error
		{
			OK,
			OpenFile,
			AlreadyCreated
		};

	public:
		TLogFile();
		~TLogFile();

		Error Create(const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting);
		void Close();
		void RecalcLevel();
		void Print(const TCHAR* format, ...);

		void Log(Type type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ...);

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

#if SUPPORT_COLOURED_LOGS
		void SetColouringMode(TBOOL a_bEnabled)
		{
			m_bColouring = a_bEnabled;
		}
#endif // SUPPORT_COLOURED_LOGS

		TINT GetTypeCount(Type type) const
		{
			return m_typeCounts[type];
		}

		TINT GetTotalLogCount() const
		{
			return m_iTotalLogCount;
		}

		const TCHAR* GetLevelString() const
		{
			return m_LevelString;
		}

	private:
		TFile* m_pFile;                 // 0x00
		TINT m_iTotalLogCount;           // 0x04
		TBOOL m_bIsSimpleMode;          // 0x08
		TBOOL m_bAllowIndentation;      // 0x09
		TCHAR m_LevelString[cLevelMax];  // 0x0A
		TUINT32 m_curLevel;            // 0x2C
		TINT m_typeCounts[Type_NUMOF];   // 0x30
		TINT* m_unk2;                    // 0x34
		TINT m_unk3;                     // 0x38

#if SUPPORT_COLOURED_LOGS
		TBOOL m_bColouring = TTRUE;
#endif // SUPPORT_COLOURED_LOGS
	};
}