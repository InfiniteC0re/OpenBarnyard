#include "ToshiPCH.h"
#include "TNativeFile_WASM.h"
#include "Toshi/Platform.h"
#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

#include <emscripten.h>
#include <emscripten/console.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

// Downloads a whole text file, the caller frees the result
EM_JS( char*, WASM_DownloadText, ( const char* a_szUrl ), {
	var szText;

	// The stored data pack has its own index
	if ( Module.oDataPack && UTF8ToString( a_szUrl ) == "index.txt" )
	{
		szText = Module.oDataPack.szIndex;
	}
	else
	{
		var oRequest = new XMLHttpRequest();
		oRequest.open( "GET", Module.szDataUrl + UTF8ToString( a_szUrl ), false );
		oRequest.send();

		if ( oRequest.status != 200 )
			return 0;

		szText = oRequest.responseText;
	}

	var iSize  = lengthBytesUTF8( szText ) + 1;
	var pText  = _malloc( iSize );
	stringToUTF8( szText, pText, iSize );
	return pText;
} );

// Downloads a part of a file. It's a blocking request, which only workers can do, and the game runs on one
EM_JS( int, WASM_DownloadRange, ( const char* a_szPath, unsigned int a_uiOffset, unsigned int a_uiSize, unsigned char* a_pDst ), {
	var szPath = UTF8ToString( a_szPath );

	// Reads of the stored data pack don't leave the device
	if ( Module.oDataPack && Module.oDataPack.oOffsets.has( szPath ) )
	{
		var iStart = Module.oDataPack.oOffsets.get( szPath ) + a_uiOffset;
		var oData  = new Uint8Array( Module.oDataPack.oReader.readAsArrayBuffer( Module.oDataPack.oFile.slice( iStart, iStart + a_uiSize ) ) );

		HEAPU8.set( oData, a_pDst );
		return oData.length;
	}

	var szUrl = Module.szDataUrl + szPath.split( "/" ).map( encodeURIComponent ).join( "/" );

	for ( var iTry = 0; iTry < 5; iTry++ )
	{
		var oRequest = new XMLHttpRequest();
		oRequest.open( "GET", szUrl, false );
		oRequest.setRequestHeader( "Range", "bytes=" + a_uiOffset + "-" + ( a_uiOffset + a_uiSize - 1 ) );
		oRequest.responseType = "arraybuffer";

		try
		{
			oRequest.send();
		}
		catch ( e )
		{
			continue;
		}

		if ( oRequest.status == 206 || oRequest.status == 200 )
		{
			var oData = new Uint8Array( oRequest.response );

			// A server without Range support sends everything
			if ( oRequest.status == 200 )
				oData = oData.subarray( a_uiOffset, a_uiOffset + a_uiSize );

			HEAPU8.set( oData, a_pDst );
			return oData.length;
		}
	}

	return 0;
} );

// Keeps a written file in the save folder, the page stores that in IndexedDB
static void WASM_SaveFile( const char* a_szPath, const unsigned char* a_pData, unsigned int a_uiSize )
{
	char szPath[ 512 ];
	snprintf( szPath, sizeof( szPath ), "/Save/%s", a_szPath );

	for ( char* p = szPath; *p; p++ )
		if ( *p == '/' && p != szPath && p - szPath > 5 )
		{
			*p = '_';
		}

	if ( FILE* pFile = WASM_fopen( szPath, "wb" ) )
	{
		fwrite( a_pData, 1, a_uiSize, pFile );
		WASM_fclose( pFile );
	}
}

static void WASM_RemoveFile( const char* a_szPath )
{
}

// Reads back a file of the save folder, TNULL if it isn't there
static unsigned char* WASM_LoadSavedFile( const char* a_szPath, unsigned int& a_ruiSize )
{
	char szPath[ 512 ];
	snprintf( szPath, sizeof( szPath ), "/Save/%s", a_szPath );

	for ( char* p = szPath; *p; p++ )
		if ( *p == '/' && p != szPath && p - szPath > 5 )
		{
			*p = '_';
		}

	FILE* pFile = WASM_fopen( szPath, "rb" );

	if ( !pFile )
		return TNULL;

	fseek( pFile, 0, SEEK_END );
	a_ruiSize = (unsigned int)ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	unsigned char* pData = (unsigned char*)malloc( a_ruiSize ? a_ruiSize : 1 );
	fread( pData, 1, a_ruiSize, pFile );
	WASM_fclose( pFile );

	return pData;
}

TOSHI_NAMESPACE_START

static constexpr TUINT32 BLOCK_SIZE   = 1024 * 1024;
static constexpr TUINT32 MAX_BLOCKS   = 32;
static constexpr TUINT32 NO_BLOCK     = TUINT32( -1 );
static constexpr TUINT32 MAX_PATH_LEN = 256;

struct WASMFileEntry
{
	TCHAR   szKey[ MAX_PATH_LEN ];  // lower case, with forward slashes
	TCHAR   szPath[ MAX_PATH_LEN ]; // as the server knows it
	TUINT32 uiSize;
	TUINT8* pWritten; // the contents if the game wrote it
	TUINT32 uiCapacity;
	TBOOL   bDirectory;
};

struct WASMFileBlock
{
	TINT    iEntry;
	TUINT32 uiBlock;
	TUINT32 uiUsed;
	TUINT32 uiLastUse;
	TUINT8* pData;
};

static WASMFileEntry* s_pEntries;
static TINT          s_iNumEntries;
static TINT          s_iCapacity;
static WASMFileBlock  s_aBlocks[ MAX_BLOCKS ];
static TUINT32       s_uiUseCounter;
static TMutex        s_oMutex;
static TBOOL         s_bIndexLoaded;

static void MakeKey( const TCHAR* a_szPath, TCHAR* a_szOutKey )
{
	TINT iOut = 0;

	for ( const TCHAR* p = a_szPath; *p && iOut < MAX_PATH_LEN - 1; p++ )
	{
		TCHAR c = *p;
		if ( c == '\\' ) c = '/';
		if ( c >= 'A' && c <= 'Z' ) c = TCHAR( c - 'A' + 'a' );

		// Paths are relative to the game folder
		if ( c == '/' && iOut == 0 ) continue;
		if ( c == '/' && a_szOutKey[ iOut - 1 ] == '/' ) continue;
		if ( c == '.' && iOut == 0 && p[ 1 ] && ( p[ 1 ] == '/' || p[ 1 ] == '\\' ) ) continue;

		a_szOutKey[ iOut++ ] = c;
	}

	while ( iOut > 0 && a_szOutKey[ iOut - 1 ] == '/' ) iOut--;
	a_szOutKey[ iOut ] = '\0';
}

static TINT FindEntry( const TCHAR* a_szKey )
{
	for ( TINT i = 0; i < s_iNumEntries; i++ )
	{
		if ( T2String8::Compare( s_pEntries[ i ].szKey, a_szKey ) == 0 )
			return i;
	}

	return -1;
}

static TINT AddEntry( const TCHAR* a_szPath, TUINT32 a_uiSize, TBOOL a_bDirectory )
{
	if ( s_iNumEntries == s_iCapacity )
	{
		s_iCapacity = s_iCapacity ? s_iCapacity * 2 : 16384;
		s_pEntries  = TSTATICCAST( WASMFileEntry, realloc( s_pEntries, s_iCapacity * sizeof( WASMFileEntry ) ) );
	}

	WASMFileEntry& rEntry = s_pEntries[ s_iNumEntries ];
	MakeKey( a_szPath, rEntry.szKey );
	T2String8::Copy( rEntry.szPath, a_szPath, MAX_PATH_LEN - 1 );
	rEntry.szPath[ MAX_PATH_LEN - 1 ] = '\0';
	rEntry.uiSize                     = a_uiSize;
	rEntry.pWritten                   = TNULL;
	rEntry.uiCapacity                 = 0;
	rEntry.bDirectory                 = a_bDirectory;

	return s_iNumEntries++;
}

// Lines of index.txt: <d or f><tab><size><tab><path>
static void LoadIndex()
{
	if ( s_bIndexLoaded )
		return;

	s_bIndexLoaded = TTRUE;
	s_oMutex.Create();

	TCHAR* pText = WASM_DownloadText( "index.txt" );

	if ( !pText )
	{
		emscripten_console_error( "TNativeFileSystem: index.txt was not found" );
		return;
	}

	for ( TCHAR* pLine = pText; *pLine; )
	{
		TCHAR* pEnd = pLine;
		while ( *pEnd && *pEnd != '\n' ) pEnd++;

		const TBOOL bLast = *pEnd == '\0';
		*pEnd             = '\0';

		if ( pEnd - pLine > 4 )
		{
			if ( pEnd[ -1 ] == '\r' ) pEnd[ -1 ] = '\0';

			TCHAR* pSize = pLine + 2;
			TCHAR* pPath = pSize;
			while ( *pPath && *pPath != '\t' ) pPath++;

			if ( *pPath == '\t' )
			{
				*pPath++ = '\0';
				AddEntry( pPath, TUINT32( strtoul( pSize, TNULL, 10 ) ), pLine[ 0 ] == 'd' );
			}
		}

		if ( bLast ) break;
		pLine = pEnd + 1;
	}

	free( pText );

	for ( TINT i = 0; i < MAX_BLOCKS; i++ )
		s_aBlocks[ i ].iEntry = -1;
}

// Copies from a cached block, -1 if it isn't cached. Needs the lock
static TINT CopyFromBlock( TINT a_iEntry, TUINT32 a_uiBlock, TUINT32 a_uiOffset, TUINT8* a_pDst, TUINT32 a_uiMax )
{
	for ( TINT i = 0; i < MAX_BLOCKS; i++ )
	{
		WASMFileBlock& rBlock = s_aBlocks[ i ];

		if ( rBlock.iEntry == a_iEntry && rBlock.uiBlock == a_uiBlock )
		{
			rBlock.uiLastUse = ++s_uiUseCounter;

			if ( a_uiOffset >= rBlock.uiUsed )
				return 0;

			const TUINT32 uiCopy = TMath::Min( a_uiMax, rBlock.uiUsed - a_uiOffset );
			TUtil::MemCopy( a_pDst, rBlock.pData + a_uiOffset, uiCopy );
			return TINT( uiCopy );
		}
	}

	return -1;
}

// Puts a downloaded block in place of the oldest one, which takes the data. Needs the lock
static void AddBlock( TINT a_iEntry, TUINT32 a_uiBlock, TUINT8* a_pData, TUINT32 a_uiUsed )
{
	WASMFileBlock* pOldest = &s_aBlocks[ 0 ];

	for ( TINT i = 0; i < MAX_BLOCKS; i++ )
	{
		WASMFileBlock& rBlock = s_aBlocks[ i ];

		// Another thread got the same block meanwhile
		if ( rBlock.iEntry == a_iEntry && rBlock.uiBlock == a_uiBlock )
		{
			free( a_pData );
			return;
		}

		if ( rBlock.iEntry == -1 || rBlock.uiLastUse < pOldest->uiLastUse )
			pOldest = &rBlock;
	}

	free( pOldest->pData );

	pOldest->iEntry    = a_iEntry;
	pOldest->uiBlock   = a_uiBlock;
	pOldest->uiLastUse = ++s_uiUseCounter;
	pOldest->uiUsed    = a_uiUsed;
	pOldest->pData     = a_pData;
}

TNativeFileSystem::TNativeFileSystem( const TCHAR* name )
    : TFileSystem( name )
{
	m_iFindEntry = -1;
	LoadIndex();
	TFileManager::GetSingletonSafe()->MountFileSystem( this );
}

TNativeFileSystem::~TNativeFileSystem()
{
	UnmountFileSystem();
}

TFile* TNativeFileSystem::CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode )
{
	TNativeFile* pFile = new TNativeFile( this );

	if ( !pFile->Open( a_rcFileName, a_eFileMode ) )
	{
		delete pFile;
		return TNULL;
	}

	return pFile;
}

void TNativeFileSystem::DestroyFile( TFile* a_pFile )
{
	if ( a_pFile != TNULL )
	{
		TSTATICCAST( TNativeFile, a_pFile )->Close();
		delete a_pFile;
	}
}

TBOOL TNativeFileSystem::RemoveFile( const TString8& a_rcFileName )
{
	TMutexLock oLock( s_oMutex );

	TCHAR szKey[ MAX_PATH_LEN ];
	MakeKey( a_rcFileName, szKey );

	const TINT iEntry = FindEntry( szKey );

	if ( iEntry < 0 )
		return TFALSE;

	s_pEntries[ iEntry ].szKey[ 0 ] = '\0';

	if ( s_pEntries[ iEntry ].pWritten )
	{
		free( s_pEntries[ iEntry ].pWritten );
		s_pEntries[ iEntry ].pWritten = TNULL;
	}

	WASM_RemoveFile( szKey );
	return TTRUE;
}

TString8 TNativeFileSystem::MakeInternalPath( const TString8& a_rcPath )
{
	TString8 strInternalPath = TFileManager::GetSingleton()->MakeAbsolutePath( a_rcPath );
	FixPathSlashesBack( strInternalPath );

	return strInternalPath;
}

TBOOL TNativeFileSystem::GetFirstFile( const TString8& a_rcPath, TString8& a_rOutFileName, TFINDFILE a_ui8Flags )
{
	TCHAR szKey[ MAX_PATH_LEN ];
	MakeKey( a_rcPath, szKey );

	m_strFindDirectory = szKey;
	m_iFindEntry       = 0;

	return GetNextFile( a_rOutFileName, a_ui8Flags );
}

TBOOL TNativeFileSystem::GetNextFile( TString8& a_rOutFileName, TFINDFILE a_ui8Flags )
{
	const TINT iDirectoryLen = m_strFindDirectory.Length();

	for ( ; m_iFindEntry >= 0 && m_iFindEntry < s_iNumEntries; m_iFindEntry++ )
	{
		const WASMFileEntry& rEntry = s_pEntries[ m_iFindEntry ];

		if ( T2String8::Compare( rEntry.szKey, m_strFindDirectory, iDirectoryLen ) != 0 )
			continue;

		const TCHAR* pName = rEntry.szKey + iDirectoryLen;

		if ( iDirectoryLen != 0 )
		{
			if ( *pName != '/' ) continue;
			pName++;
		}

		if ( *pName == '\0' || T2String8::FindChar( pName, '/' ) )
			continue;

		if ( ( a_ui8Flags & ( rEntry.bDirectory ? TFINDFILE_DIRECTORY : TFINDFILE_FILE ) ) == 0 )
			continue;

		// Keep the case of the name
		a_rOutFileName = T2String8::Length( rEntry.szPath ) >= iDirectoryLen ? rEntry.szPath + ( pName - rEntry.szKey ) : pName;
		m_iFindEntry++;

		return TTRUE;
	}

	return TFALSE;
}

TBOOL TNativeFileSystem::MakeDirectory( const TString8& a_rcDirectory )
{
	return TTRUE;
}

TNativeFile::TNativeFile( TNativeFileSystem* a_pFileSystem )
    : TFile( a_pFileSystem )
{
	m_iEntry     = -1;
	m_uiPosition = 0;
	m_bWriting   = TFALSE;
	m_bDirty     = TFALSE;
}

TNativeFile::~TNativeFile()
{
}

TBOOL TNativeFile::Open( const TString8& a_FileName, TFILEMODE a_Mode )
{
	TMutexLock oLock( s_oMutex );

	TCHAR szKey[ MAX_PATH_LEN ];
	MakeKey( a_FileName, szKey );

	m_iEntry   = FindEntry( szKey );
	m_bWriting = HASANYFLAG( a_Mode, TFILEMODE_WRITE | TFILEMODE_READWRITE | TFILEMODE_CREATENEW );

	if ( m_iEntry < 0 )
	{
		TUINT32 uiSavedSize = 0;
		TUINT8* pSaved      = WASM_LoadSavedFile( szKey, uiSavedSize );

		if ( pSaved )
		{
			m_iEntry                          = AddEntry( szKey, uiSavedSize, TFALSE );
			s_pEntries[ m_iEntry ].pWritten   = pSaved;
			s_pEntries[ m_iEntry ].uiCapacity = uiSavedSize;
		}
		else if ( !HASANYFLAG( a_Mode, TFILEMODE_CREATENEW ) )
		{
			return TFALSE;
		}
		else
		{
			m_iEntry = AddEntry( szKey, 0, TFALSE );
		}
	}

	if ( s_pEntries[ m_iEntry ].bDirectory )
		return TFALSE;

	if ( HASANYFLAG( a_Mode, TFILEMODE_CREATENEW ) )
	{
		s_pEntries[ m_iEntry ].uiSize = 0;
		m_bDirty                      = TTRUE;
	}

	return TTRUE;
}

void TNativeFile::Close()
{
	FlushBuffers();
	m_iEntry = -1;
}

void TNativeFile::FlushBuffers()
{
	if ( m_iEntry >= 0 && m_bDirty )
	{
		TMutexLock oLock( s_oMutex );

		const WASMFileEntry& rEntry = s_pEntries[ m_iEntry ];

		if ( rEntry.pWritten )
			WASM_SaveFile( rEntry.szKey, rEntry.pWritten, rEntry.uiSize );

		m_bDirty = TFALSE;
	}
}

TSIZE TNativeFile::Read( void* a_pDst, TSIZE a_uiSize )
{
	// Downloads run without the lock: a long read on one thread mustn't stall the others (the sound mixer streams music)
	TCHAR   szPath[ MAX_PATH_LEN ];
	TUINT32 uiFileSize;
	TUINT32 uiToRead;

	{
		TMutexLock oLock( s_oMutex );

		const WASMFileEntry& rEntry = s_pEntries[ m_iEntry ];

		if ( m_uiPosition >= rEntry.uiSize )
			return 0;

		uiToRead = TMath::Min( TUINT32( a_uiSize ), rEntry.uiSize - m_uiPosition );

		if ( rEntry.pWritten )
		{
			TUtil::MemCopy( a_pDst, rEntry.pWritten + m_uiPosition, uiToRead );
			m_uiPosition += uiToRead;
			return uiToRead;
		}

		T2String8::Copy( szPath, rEntry.szPath, MAX_PATH_LEN - 1 );
		szPath[ MAX_PATH_LEN - 1 ] = '\0';
		uiFileSize                 = rEntry.uiSize;
	}

	TUINT8* pDst  = TSTATICCAST( TUINT8, a_pDst );
	TUINT32 uiLeft = uiToRead;

	while ( uiLeft > 0 )
	{
		const TUINT32 uiBlock  = m_uiPosition / BLOCK_SIZE;
		const TUINT32 uiOffset = m_uiPosition % BLOCK_SIZE;

		// Big reads skip the cache
		if ( uiOffset == 0 && uiLeft >= BLOCK_SIZE )
		{
			const TUINT32 uiWhole = uiLeft - uiLeft % BLOCK_SIZE;
			const TUINT32 uiGot   = WASM_DownloadRange( szPath, m_uiPosition, uiWhole, pDst );

			if ( uiGot != uiWhole )
			{
				TERROR( "TNativeFile: couldn't download %s\n", szPath );
				break;
			}

			pDst += uiWhole;
			m_uiPosition += uiWhole;
			uiLeft -= uiWhole;
			continue;
		}

		TINT iCopied;

		{
			TMutexLock oLock( s_oMutex );
			iCopied = CopyFromBlock( m_iEntry, uiBlock, uiOffset, pDst, uiLeft );
		}

		if ( iCopied < 0 )
		{
			const TUINT32 uiBlockStart = uiBlock * BLOCK_SIZE;
			const TUINT32 uiSize       = TMath::Min( uiFileSize - uiBlockStart, BLOCK_SIZE );

			TUINT8*       pData  = TSTATICCAST( TUINT8, malloc( BLOCK_SIZE ) );
			const TUINT32 uiUsed = WASM_DownloadRange( szPath, uiBlockStart, uiSize, pData );

			if ( uiUsed != uiSize )
				TERROR( "TNativeFile: couldn't download %s (%u bytes at %u)\n", szPath, uiSize, uiBlockStart );

			iCopied = uiOffset < uiUsed ? TINT( TMath::Min( uiLeft, uiUsed - uiOffset ) ) : 0;
			TUtil::MemCopy( pDst, pData + uiOffset, TUINT32( iCopied ) );

			TMutexLock oLock( s_oMutex );
			AddBlock( m_iEntry, uiBlock, pData, uiUsed );
		}

		if ( iCopied == 0 )
			break;

		pDst += iCopied;
		m_uiPosition += iCopied;
		uiLeft -= iCopied;
	}

	return uiToRead - uiLeft;
}

TSIZE TNativeFile::Write( const void* a_pSrc, TSIZE a_uiSize )
{
	TMutexLock oLock( s_oMutex );

	WASMFileEntry& rEntry = s_pEntries[ m_iEntry ];

	const TUINT32 uiEnd = m_uiPosition + TUINT32( a_uiSize );

	if ( !rEntry.pWritten || uiEnd > rEntry.uiCapacity )
	{
		const TUINT32 uiCapacity = TMath::Max( uiEnd * 2, 4096u );
		TUINT8*       pNew       = TSTATICCAST( TUINT8, malloc( uiCapacity ) );

		// A file from the server is copied before it's changed
		if ( rEntry.pWritten )
		{
			TUtil::MemCopy( pNew, rEntry.pWritten, rEntry.uiSize );
			free( rEntry.pWritten );
		}
		else
		{
			for ( TUINT32 uiOffset = 0; uiOffset < rEntry.uiSize; )
			{
				const TUINT32 uiPart = TMath::Min( rEntry.uiSize - uiOffset, BLOCK_SIZE );
				WASM_DownloadRange( rEntry.szPath, uiOffset, uiPart, pNew + uiOffset );
				uiOffset += uiPart;
			}
		}

		rEntry.pWritten   = pNew;
		rEntry.uiCapacity = uiCapacity;
	}

	TUtil::MemCopy( rEntry.pWritten + m_uiPosition, a_pSrc, a_uiSize );

	m_uiPosition  = uiEnd;
	rEntry.uiSize = TMath::Max( rEntry.uiSize, uiEnd );
	m_bDirty      = TTRUE;

	return a_uiSize;
}

TBOOL TNativeFile::Seek( TINT a_iOffset, TSEEK a_eOrigin )
{
	TINT iPosition;

	switch ( a_eOrigin )
	{
		case TSEEK_SET: iPosition = a_iOffset; break;
		case TSEEK_CUR: iPosition = TINT( m_uiPosition ) + a_iOffset; break;
		case TSEEK_END: iPosition = TINT( GetSize() ) + a_iOffset; break;
		default: return TFALSE;
	}

	if ( iPosition < 0 )
		return TFALSE;

	m_uiPosition = TUINT32( iPosition );
	return TTRUE;
}

TSIZE TNativeFile::Tell()
{
	return m_uiPosition;
}

TSIZE TNativeFile::GetSize()
{
	return m_iEntry >= 0 ? s_pEntries[ m_iEntry ].uiSize : 0;
}

TUINT64 TNativeFile::GetDate()
{
	return 0;
}

TCHAR TNativeFile::GetCChar()
{
	TCHAR cCharacter = -1;
	Read( &cCharacter, sizeof( cCharacter ) );
	return cCharacter;
}

TWCHAR TNativeFile::GetWChar()
{
	TWCHAR cCharacter = 0xFFFF;
	Read( &cCharacter, sizeof( cCharacter ) );
	return cCharacter;
}

TINT TNativeFile::PutCChar( TCHAR a_cCharacter )
{
	return Write( &a_cCharacter, sizeof( a_cCharacter ) ) == sizeof( a_cCharacter ) ? TUINT8( a_cCharacter ) : -1;
}

TINT TNativeFile::PutWChar( TWCHAR a_wcCharacter )
{
	return Write( &a_wcCharacter, sizeof( a_wcCharacter ) ) == sizeof( a_wcCharacter ) ? a_wcCharacter : -1;
}

TINT TNativeFile::VCPrintf( const TCHAR* a_szFormat, va_list a_vargs )
{
	TCHAR szBuffer[ 512 ];
	const TINT iLength = vsnprintf( szBuffer, sizeof( szBuffer ), a_szFormat, a_vargs );

	if ( iLength > 0 )
		Write( szBuffer, TMath::Min( iLength, TINT( sizeof( szBuffer ) - 1 ) ) );

	return iLength;
}

TINT TNativeFile::VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs )
{
	TWCHAR wszBuffer[ 512 ];
	const TINT iLength = T2String16::FormatV( wszBuffer, TARRAYSIZE( wszBuffer ), a_wszFormat, a_vargs );

	if ( iLength > 0 )
		Write( wszBuffer, iLength * sizeof( TWCHAR ) );

	return iLength;
}

TOSHI_NAMESPACE_END
