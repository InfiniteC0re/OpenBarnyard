#pragma once
#include "TFile.h"

TOSHI_NAMESPACE_START

class TNullFileSystem;

//-----------------------------------------------------------------------------
// Purpose: File that discards everything written to it and has no content.
//-----------------------------------------------------------------------------
class TNullFile : public TFile
{
public:
	friend TNullFileSystem;

public:
	virtual TSIZE  Read( void* a_pDst, TSIZE a_uiSize ) OVERRIDE;                                 // +0x00
	virtual TSIZE  Write( const void* a_pSrc, TSIZE a_uiSize ) OVERRIDE;                          // +0x04
	virtual TBOOL  Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) OVERRIDE { return TTRUE; } // +0x08, RETURNS_TRUE 00558f80
	virtual TSIZE  Tell() OVERRIDE { return 0; }                                                  // +0x0C, RETURNS_ZERO 0054d8b0
	virtual TSIZE  GetSize() OVERRIDE { return 0; }                                               // +0x14, RETURNS_ZERO 0054d8b0
	virtual TCHAR  GetCChar() OVERRIDE;                                                           // +0x1C
	virtual TWCHAR GetWChar() OVERRIDE;                                                           // +0x20
	virtual TINT   PutCChar( TCHAR a_cCharacter ) OVERRIDE;                                       // +0x24
	virtual TINT   PutWChar( TWCHAR a_wcCharacter ) OVERRIDE;                                     // +0x28
	virtual TINT   VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) OVERRIDE;                 // +0x2C
	virtual TINT   VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) OVERRIDE;               // +0x30
	virtual ~TNullFile();                                                                         // +0x34

	TINT GetBytesWritten() const { return m_iBytesWritten; }

protected:
	TNullFile( TNullFileSystem* a_pFileSystem );

private:
	TINT m_iUnk1;         // 0x08 (never accessed)
	TINT m_iBytesWritten; // 0x0C
};

class TNullFileSystem : public TFileSystem
{
public:
	TNullFileSystem( const TCHAR* a_szName );
	virtual ~TNullFileSystem();

	virtual TFile*   CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) OVERRIDE;
	virtual void     DestroyFile( TFile* a_pFile ) OVERRIDE;
	virtual TString8 MakeInternalPath( const TString8& a_rcPath ) OVERRIDE;

	static TNullFileSystem* GetNullFileSystem() { return s_pFileSystem; }

private:
	inline static TNullFileSystem* s_pFileSystem;
};

TOSHI_NAMESPACE_END
