#pragma once
#include "File/TFile.h"

#undef CreateFile
#undef FindFirstFile

namespace Toshi {

	class TNativeFileSystem : public TFileSystem
	{
	public:
		TNativeFileSystem( const TCHAR* name );
		virtual ~TNativeFileSystem();

		virtual TFile* CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) OVERRIDE;
		virtual void DestroyFile( TFile* a_pFile ) OVERRIDE;
		virtual TBOOL RemoveFile( const TString8& a_rcFileName ) OVERRIDE;
		virtual TString8 MakeInternalPath( const TString8& a_rcPath ) OVERRIDE;
		virtual TBOOL GetFirstFile( const TString8& a_rcPath, TString8& a_rOutFileName, TUINT8 a_ui8Flags ) OVERRIDE;
		virtual TBOOL GetNextFile( TString8& a_rOutFileName, TUINT8 a_ui8Flags ) OVERRIDE;

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
		virtual TUINT Read( void* a_pDst, TUINT a_uiSize ) OVERRIDE;
		virtual TUINT Write( const void* a_pSrc, TUINT a_uiSize ) OVERRIDE;
		virtual TBOOL Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) OVERRIDE;
		virtual TUINT Tell() OVERRIDE;
		virtual TUINT GetSize() OVERRIDE;
		virtual _FILETIME GetDate() OVERRIDE;
		virtual TCHAR GetCChar() OVERRIDE;
		virtual TWCHAR GetWChar() OVERRIDE;
		virtual TINT PutCChar( TCHAR a_cCharacter ) OVERRIDE;
		virtual TINT PutWChar( TWCHAR a_wcCharacter ) OVERRIDE;
		virtual TINT CPrintf( const TCHAR* a_szFormat, ... ) OVERRIDE;
		virtual TINT WPrintf( const TWCHAR* a_wszFormat, ... ) OVERRIDE;
		virtual TINT VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) OVERRIDE;
		virtual TINT VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) OVERRIDE;

		TBOOL Open( const TString8& a_FileName, TFILEMODE a_Mode );

		void Close();

	private:
		TBOOL LoadBuffer( DWORD bufferPos );
		TINT FlushWriteBuffer();
		TINT ReadUnbuffered( LPVOID dst, TUINT size );

	private:
		HANDLE m_Handle;             // 0x8
		DWORD m_Position;            // 0xC
		DWORD m_RBufferPosition;     // 0x10
		DWORD m_PrevBufferPos;       // 0x14
		DWORD m_LastBufferSize;      // 0x18
		TCHAR* m_RBuffer;            // 0x1C (read buffer)
		TCHAR* m_WBuffer;            // 0x20 (write buffer)
		DWORD m_WriteBufferUsed;     // 0x24
		TBOOL m_WriteBuffered;       // 0x28
	};

}


