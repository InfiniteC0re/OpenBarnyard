#pragma once
#include "TFile.h"

TOSHI_NAMESPACE_START

class TMemFileSystem;

//-----------------------------------------------------------------------------
// Purpose: Read-only file whose whole content is loaded into memory from the
// "local" file system when opened.
//-----------------------------------------------------------------------------
class TMemFile : public TFile
{
public:
	friend TMemFileSystem;

public:
	virtual TSIZE  Read( void* a_pDst, TSIZE a_uiSize ) OVERRIDE;                   // +0x00
	virtual TSIZE  Write( const void* a_pSrc, TSIZE a_uiSize ) OVERRIDE;            // +0x04
	virtual TBOOL  Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) OVERRIDE;    // +0x08
	virtual TSIZE  Tell() OVERRIDE;                                                 // +0x0C
	virtual TSIZE  GetSize() OVERRIDE { return m_uiSize; }                          // +0x14, folded getter 004ffb50
	virtual TCHAR  GetCChar() OVERRIDE;                                             // +0x1C
	virtual TWCHAR GetWChar() OVERRIDE;                                             // +0x20
	virtual TINT   PutCChar( TCHAR a_cCharacter ) OVERRIDE;                         // +0x24
	virtual TINT   PutWChar( TWCHAR a_wcCharacter ) OVERRIDE;                       // +0x28
	virtual TINT   VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) OVERRIDE;   // +0x2C
	virtual TINT   VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) OVERRIDE; // +0x30
	virtual ~TMemFile();                                                            // +0x34
	virtual void DebugStats() {}                                                    // +0x38, DOES_NOTHING
	virtual void ClearStats() {}                                                    // +0x3C, DOES_NOTHING

protected:
	TMemFile( TMemFileSystem* a_pFileSystem );

	TBOOL Open( const TString8& a_rcFileName, TFILEMODE a_eFileMode );

private:
	TUINT8* m_pData;   // 0x08
	TUINT8* m_pCursor; // 0x0C
	TSIZE   m_uiSize;  // 0x10
};

class TMemFileSystem : public TFileSystem
{
public:
	TMemFileSystem( const TCHAR* a_szName );
	virtual ~TMemFileSystem();

	virtual TFile*   CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) OVERRIDE;
	virtual void     DestroyFile( TFile* a_pFile ) OVERRIDE;
	virtual TString8 MakeInternalPath( const TString8& a_rcPath ) OVERRIDE;

private:
	// NOTE: in retail this is at 0x24 since TFileSystem doesn't have m_NextFileHandle
	TFileSystem* m_pLocalFileSystem;
};

TOSHI_NAMESPACE_END
