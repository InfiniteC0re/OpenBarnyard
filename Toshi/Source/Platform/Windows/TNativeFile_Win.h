#pragma once
#include "File/TFile.h"

TOSHI_NAMESPACE_START

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
};

class TNativeFile : public TFile
{
public:
	static constexpr TUINT32 BUFFER_SIZE = 0x800;

public:
	TNativeFile( TNativeFileSystem* a_pFileSystem );
	TNativeFile( const TNativeFile& other );

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
	virtual TINT    CPrintf( const TCHAR* a_szFormat, ... ) OVERRIDE;
	virtual TINT    WPrintf( const TWCHAR* a_wszFormat, ... ) OVERRIDE;

	TBOOL Open( const TString8& a_FileName, TFILEMODE a_Mode );

	void Close();

private:
	TBOOL LoadBuffer( TUINT32 bufferPos );
	TINT  FlushWriteBuffer();
	TINT  ReadUnbuffered( void* dst, TUINT size );

protected:
	void*   m_Handle;          // 0x8
	TUINT32 m_Position;        // 0xC
	TUINT32 m_RBufferPosition; // 0x10
	TUINT32 m_PrevBufferPos;   // 0x14
	TUINT32 m_LastBufferSize;  // 0x18
	TCHAR*  m_RBuffer;         // 0x1C (read buffer)
	TCHAR*  m_WBuffer;         // 0x20 (write buffer)
	TUINT32 m_WriteBufferUsed; // 0x24
	TBOOL   m_WriteBuffered;   // 0x28
};

TOSHI_NAMESPACE_END
