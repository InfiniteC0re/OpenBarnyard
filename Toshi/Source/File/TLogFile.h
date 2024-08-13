#pragma once

TOSHI_NAMESPACE_START

class TFile;

class TLogFile
{
private:
	static constexpr TUINT32 cLevelMax = 30;

public:
	enum class Error
	{
		OK,
		OpenFile,
		AlreadyCreated
	};

public:
	TLogFile();
	~TLogFile();

	Error Create( const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting );
	void  Close();

	void RecalcLevel();

	void Print( const TCHAR* format, ... );
	void Log( TUtil::LogType type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ... );

	void Down();
	void Up();

	void  AllowIndentation( TBOOL a_bAllow ) { m_bAllowIndentation = a_bAllow; }
	TBOOL GetAllowIndentation() { return m_bAllowIndentation; }

	void  SetSimpleMode( TBOOL a_bEnable ) { m_bIsSimpleMode = a_bEnable; }
	TBOOL IsSimpleMode() { return m_bIsSimpleMode; }

	TINT		 GetTypeCount( TUtil::LogType a_eType ) const { return m_typeCounts[ a_eType ]; }
	TINT		 GetTotalLogCount() const { return m_iTotalLogCount; }
	const TCHAR* GetLevelString() const { return m_LevelString; }

private:
	TFile*	m_pFile;							  // 0x00
	TINT	m_iTotalLogCount;					  // 0x04
	TBOOL	m_bIsSimpleMode;					  // 0x08
	TBOOL	m_bAllowIndentation;				  // 0x09
	TCHAR	m_LevelString[ cLevelMax ];			  // 0x0A
	TUINT32 m_curLevel;							  // 0x2C
	TINT	m_typeCounts[ TUtil::LogType_NUMOF ]; // 0x30
	TINT*	m_unk2;								  // 0x34
	TINT	m_unk3;								  // 0x38
};

TOSHI_NAMESPACE_END
