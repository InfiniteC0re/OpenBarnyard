#pragma once
#include "File/TFile.h"

TOSHI_NAMESPACE_START

// The game's files live on a static server: index.txt lists them and they're read in blocks with Range requests.
// Files the game writes stay in memory, the page keeps a copy in IndexedDB.
class TNativeFileSystem : public TFileSystem
{
public:
	TNativeFileSystem( const TCHAR* name );
	virtual ~TNativeFileSystem();

	virtual TFile*   CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) OVERRIDE;
	virtual void     DestroyFile( TFile* a_pFile ) OVERRIDE;
	virtual TBOOL    RemoveFile( const TString8& a_rcFileName ) OVERRIDE;
	virtual TString8 MakeInternalPath( const TString8& a_rcPath ) OVERRIDE;
	virtual TBOOL    GetFirstFile( const TString8& a_rcPath, TString8& a_rOutFileName, TFINDFILE a_ui8Flags ) OVERRIDE;
	virtual TBOOL    GetNextFile( TString8& a_rOutFileName, TFINDFILE a_ui8Flags ) OVERRIDE;

	virtual TBOOL MakeDirectory( const TString8& a_rcDirectory );

private:
	TString8 m_strFindDirectory;
	TINT     m_iFindEntry;
};

class TNativeFile : public TFile
{
public:
	TNativeFile( TNativeFileSystem* a_pFileSystem );
	~TNativeFile();

	//-----------------------------------------------------------------------------
	// TFile
	//-----------------------------------------------------------------------------
	virtual TSIZE   Read( void* a_pDst, TSIZE a_uiSize ) OVERRIDE;
	virtual TSIZE   Write( const void* a_pSrc, TSIZE a_uiSize ) OVERRIDE;
	virtual TBOOL   Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) OVERRIDE;
	virtual TSIZE   Tell() OVERRIDE;
	virtual void    FlushBuffers() OVERRIDE;
	virtual TSIZE   GetSize() OVERRIDE;
	virtual TUINT64 GetDate() OVERRIDE;
	virtual TCHAR   GetCChar() OVERRIDE;
	virtual TWCHAR  GetWChar() OVERRIDE;
	virtual TINT    PutCChar( TCHAR a_cCharacter ) OVERRIDE;
	virtual TINT    PutWChar( TWCHAR a_wcCharacter ) OVERRIDE;
	virtual TINT    VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) OVERRIDE;
	virtual TINT    VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) OVERRIDE;

	TBOOL Open( const TString8& a_FileName, TFILEMODE a_Mode );
	void  Close();

protected:
	TINT    m_iEntry;
	TUINT32 m_uiPosition;
	TBOOL   m_bWriting;
	TBOOL   m_bDirty;
};

TOSHI_NAMESPACE_END
