#include "ToshiPCH.h"
#include "TShaderCombo_DX11.h"
#include "TRenderInterface_DX11.h"

#include "Thread/TMutex.h"
#include "Thread/TMutexLock.h"

#include <d3dcompiler.h>
#include <d3dcommon.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// No debug info: it embeds the sources in every permutation
static constexpr UINT SHADER_COMPILE_FLAGS = D3DCOMPILE_OPTIMIZATION_LEVEL3;

static TUINT64 HashData( const void* a_pData, TSIZE a_uiSize, TUINT64 a_uiHash = 14695981039346656037ull )
{
	const TUINT8* pBytes = TSTATICCAST( const TUINT8, a_pData );

	for ( TSIZE i = 0; i < a_uiSize; i++ )
	{
		a_uiHash ^= pBytes[ i ];
		a_uiHash *= 1099511628211ull;
	}

	return a_uiHash;
}

static TUINT64 HashString( const TCHAR* a_szString, TUINT64 a_uiHash )
{
	return HashData( a_szString, T2String8::Length( a_szString ) + 1, a_uiHash );
}

// The source files read while compiling one entry point, every permutation includes the same ones.
// The permutations compile on several threads, so it's read with Win32 and guarded by a mutex
struct TShaderSourceFile
{
	TString8 strFileName;
	TCHAR*   pchData;
	TUINT    uiSize;
	TUINT64  uiHash;
};

class TShaderSourceCache
{
public:
	static constexpr TUINT MAX_FILES = 32;

	using File = TShaderSourceFile;

public:
	TShaderSourceCache()
	    : m_uiNumFiles( 0 )
	{
		m_oMutex.Create();
	}

	~TShaderSourceCache()
	{
		for ( TUINT i = 0; i < m_uiNumFiles; i++ )
			delete[] m_aFiles[ i ].pchData;
	}

	const File* Get( const TString8& a_rcFileName )
	{
		TMUTEX_LOCK_SCOPE( m_oMutex );

		for ( TUINT i = 0; i < m_uiNumFiles; i++ )
		{
			if ( m_aFiles[ i ].strFileName.CompareNoCase( a_rcFileName.GetString() ) == 0 )
				return &m_aFiles[ i ];
		}

		if ( m_uiNumFiles == MAX_FILES )
			return TNULL;

		HANDLE hFile = CreateFileA( a_rcFileName, GENERIC_READ, FILE_SHARE_READ, TNULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, TNULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			return TNULL;

		const DWORD uiSize  = GetFileSize( hFile, TNULL );
		TCHAR*      pchData = new TCHAR[ uiSize + 1 ];
		DWORD       uiRead  = 0;
		const BOOL  bRead   = ReadFile( hFile, pchData, uiSize, &uiRead, TNULL ) && uiRead == uiSize;
		CloseHandle( hFile );

		if ( !bRead )
		{
			delete[] pchData;
			return TNULL;
		}

		pchData[ uiSize ] = '\0';

		File& rFile       = m_aFiles[ m_uiNumFiles++ ];
		rFile.strFileName = a_rcFileName;
		rFile.pchData     = pchData;
		rFile.uiSize      = uiSize;
		rFile.uiHash      = HashData( pchData, uiSize );

		return &rFile;
	}

	// Only once the compilation is over, the files can't change then
	TUINT       GetNumFiles() const { return m_uiNumFiles; }
	const File& GetFile( TUINT a_uiIndex ) const { return m_aFiles[ a_uiIndex ]; }

private:
	TMutex m_oMutex;
	File   m_aFiles[ MAX_FILES ];
	TUINT  m_uiNumFiles;
};

static TString8 GetDirectory( const TString8& a_rcFileName )
{
	const TINT iSeparator = TMath::Max( a_rcFileName.FindReverse( '/' ), a_rcFileName.FindReverse( '\\' ) );
	return ( iSeparator == -1 ) ? TString8() : a_rcFileName.Mid( 0, iSeparator + 1 );
}

// Resolves "#include" against the directory of the including file, then of the main file
class TShaderIncludeHandler : public ID3DInclude
{
public:
	TShaderIncludeHandler( TShaderSourceCache& a_rCache, const TCHAR* a_szFileName )
	    : m_rCache( a_rCache ), m_strRootDirectory( GetDirectory( a_szFileName ) ), m_uiDepth( 0 )
	{
	}

	HRESULT STDMETHODCALLTYPE Open( D3D_INCLUDE_TYPE a_eIncludeType, LPCSTR a_szFileName, LPCVOID a_pParentData, LPCVOID* a_ppData, UINT* a_puiBytes ) override
	{
		if ( T2String8::CompareNoCase( a_szFileName, TShaderCombo::COMMON_INCLUDE_NAME ) == 0 )
		{
			*a_ppData   = TRenderD3DInterface::GetCommonShaderSource();
			*a_puiBytes = UINT( T2String8::Length( TRenderD3DInterface::GetCommonShaderSource() ) );
			return Push( m_strRootDirectory );
		}

		const TString8& rcDirectory = ( m_uiDepth != 0 ) ? m_astrDirectories[ m_uiDepth - 1 ] : m_strRootDirectory;

		TString8                         strFileName = rcDirectory + a_szFileName;
		const TShaderSourceFile* pFile       = m_rCache.Get( strFileName );

		if ( !pFile )
		{
			strFileName = m_strRootDirectory + a_szFileName;
			pFile       = m_rCache.Get( strFileName );

			if ( !pFile )
				return E_FAIL;
		}

		*a_ppData   = pFile->pchData;
		*a_puiBytes = pFile->uiSize;
		return Push( GetDirectory( strFileName ) );
	}

	HRESULT STDMETHODCALLTYPE Close( LPCVOID a_pData ) override
	{
		if ( m_uiDepth != 0 )
			m_uiDepth--;

		return S_OK;
	}

private:
	HRESULT Push( const TString8& a_rcDirectory )
	{
		if ( m_uiDepth == MAX_DEPTH )
			return E_FAIL;

		m_astrDirectories[ m_uiDepth++ ] = a_rcDirectory;
		return S_OK;
	}

private:
	static constexpr TUINT MAX_DEPTH = 16;

	TShaderSourceCache& m_rCache;
	TString8            m_strRootDirectory;
	TString8            m_astrDirectories[ MAX_DEPTH ];
	TUINT               m_uiDepth;
};

// Checks the definitions of a generated header against the "// STATIC:" lines of the source
static TBOOL ValidateCombos( const TCHAR* a_szFileName, const TCHAR* a_pchSource, const TShaderCombo::Definition* a_pCombos, TUINT a_uiNumCombos )
{
	TUINT        uiNumFound = 0;
	const TCHAR* pchLine    = a_pchSource;

	while ( *pchLine )
	{
		const TCHAR* pchEnd = pchLine;
		while ( *pchEnd && *pchEnd != '\n' ) pchEnd++;

		const TCHAR* pch = pchLine;
		while ( pch < pchEnd && ( *pch == ' ' || *pch == '\t' || *pch == '\r' ) ) pch++;

		if ( pch != pchEnd )
		{
			// The declarations end at the first line of code
			if ( pch[ 0 ] != '/' || pch[ 1 ] != '/' )
				break;

			TCHAR szName[ 64 ];
			TINT  iMin, iMax;
			TCHAR szStage[ 4 ] = {};

			if ( sscanf_s( pch, "// STATIC: \"%63[^\"]\" \"%d..%d\" [%3[a-z]]", szName, TUINT( sizeof( szName ) ), &iMin, &iMax, szStage, TUINT( sizeof( szStage ) ) ) >= 3 )
			{
				const TShaderCombo::STAGE eStages = ( T2String8::Compare( szStage, "vs" ) == 0 ) ? TShaderCombo::STAGE_VS :
				                                    ( T2String8::Compare( szStage, "ps" ) == 0 ) ? TShaderCombo::STAGE_PS :
				                                    ( T2String8::Compare( szStage, "cs" ) == 0 ) ? TShaderCombo::STAGE_CS :
				                                                                                   TShaderCombo::STAGE_ALL;

				if ( uiNumFound >= a_uiNumCombos )
					break;

				const TShaderCombo::Definition& rcCombo = a_pCombos[ uiNumFound++ ];

				if ( T2String8::Compare( rcCombo.szName, szName ) != 0 || rcCombo.iMinValue != iMin || rcCombo.iMaxValue != iMax || rcCombo.eStages != eStages )
				{
					TERROR( "TShaderCombo: '%s' declares %s %d..%d, the header expects %s %d..%d, regenerate it\n", a_szFileName, szName, iMin, iMax, rcCombo.szName, rcCombo.iMinValue, rcCombo.iMaxValue );
					return TFALSE;
				}
			}
		}

		pchLine = *pchEnd ? pchEnd + 1 : pchEnd;
	}

	if ( uiNumFound != a_uiNumCombos )
	{
		TERROR( "TShaderCombo: '%s' declares %u combos, the header expects %u, regenerate it\n", a_szFileName, uiNumFound, a_uiNumCombos );
		return TFALSE;
	}

	return TTRUE;
}

static ID3DBlob* CompilePermutation( TShaderSourceCache& a_rCache, const TCHAR* a_szFileName, const TShaderSourceFile& a_rcSource, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const TShaderCombo::Definition* a_pCombos, TUINT a_uiNumCombos, TUINT a_uiPermutation )
{
	constexpr TUINT MAX_COMBOS = 32;
	TASSERT( a_uiNumCombos <= MAX_COMBOS );

	D3D_SHADER_MACRO aDefines[ MAX_COMBOS + 1 ];
	TCHAR            aValues[ MAX_COMBOS ][ 12 ];

	for ( TUINT i = 0; i < a_uiNumCombos; i++ )
	{
		const TShaderCombo::Definition& rcCombo = a_pCombos[ i ];
		const TINT iValue = rcCombo.iMinValue + TINT( ( a_uiPermutation / rcCombo.uiStride ) % TUINT( rcCombo.iMaxValue - rcCombo.iMinValue + 1 ) );

		T2String8::Format( aValues[ i ], TINT( sizeof( aValues[ i ] ) ), "%d", iValue );
		aDefines[ i ].Name       = rcCombo.szName;
		aDefines[ i ].Definition = aValues[ i ];
	}

	aDefines[ a_uiNumCombos ] = { TNULL, TNULL };

	TShaderIncludeHandler oIncludeHandler( a_rCache, a_szFileName );
	ID3DBlob*             pBlob      = TNULL;
	ID3DBlob*             pErrorBlob = TNULL;

	if ( FAILED( D3DCompile( a_rcSource.pchData, a_rcSource.uiSize, a_szFileName, aDefines, &oIncludeHandler, a_szEntryPoint, a_szTarget, SHADER_COMPILE_FLAGS, 0, &pBlob, &pErrorBlob ) ) )
	{
		const TCHAR* szError = pErrorBlob ? TSTATICCAST( const TCHAR, pErrorBlob->GetBufferPointer() ) : "";
		OutputDebugStringA( szError );
		TERROR( "TShaderCombo: %s:%s permutation %u failed to compile: %s\n", a_szFileName, a_szEntryPoint, a_uiPermutation, szError );
		pBlob = TNULL;
	}

	if ( pErrorBlob )
		pErrorBlob->Release();

	return pBlob;
}

// The debug info of the bytecode holds the defines, so it's compared without it
static ID3DBlob* StripBytecode( ID3DBlob* a_pBlob )
{
	ID3DBlob* pStripped = TNULL;

	if ( FAILED( D3DStripShader( a_pBlob->GetBufferPointer(), a_pBlob->GetBufferSize(), D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS, &pStripped ) ) )
	{
		a_pBlob->AddRef();
		return a_pBlob;
	}

	return pStripped;
}

static TBOOL IsSameBytecode( ID3DBlob* a_pBlob1, ID3DBlob* a_pBlob2 )
{
	return a_pBlob1->GetBufferSize() == a_pBlob2->GetBufferSize() &&
	    TUtil::MemCompare( a_pBlob1->GetBufferPointer(), a_pBlob2->GetBufferPointer(), a_pBlob1->GetBufferSize() ) == 0;
}

// The permutations that have to be compiled, shared by the compiler threads
struct TCompileJob
{
	TShaderSourceCache*             pCache;
	const TCHAR*                    szFileName;
	const TShaderSourceFile* pSource;
	LPCSTR                          szEntryPoint;
	LPCSTR                          szTarget;
	const TShaderCombo::Definition* pCombos;
	TUINT                           uiNumCombos;
	const TUINT*                    pPermutations;
	ID3DBlob**                      ppBlobs; // indexed like pPermutations
	TUINT                           uiNumPermutations;
	volatile LONG                   iNext;
	volatile LONG                   bFailed;
};

static DWORD WINAPI CompileThreadProc( LPVOID a_pJob )
{
	TCompileJob* pJob = TSTATICCAST( TCompileJob, a_pJob );

	while ( !pJob->bFailed )
	{
		const LONG iIndex = InterlockedIncrement( &pJob->iNext ) - 1;
		if ( iIndex >= LONG( pJob->uiNumPermutations ) )
			break;

		pJob->ppBlobs[ iIndex ] = CompilePermutation( *pJob->pCache, pJob->szFileName, *pJob->pSource, pJob->szEntryPoint, pJob->szTarget, pJob->pCombos, pJob->uiNumCombos, pJob->pPermutations[ iIndex ] );

		if ( !pJob->ppBlobs[ iIndex ] )
			InterlockedExchange( &pJob->bFailed, TRUE );
	}

	return 0;
}

static void RunCompileJob( TCompileJob& a_rJob )
{
	constexpr TUINT MAX_THREADS = 32;

	SYSTEM_INFO oSystemInfo;
	GetSystemInfo( &oSystemInfo );

	// The calling thread compiles too
	const TUINT uiNumThreads = TMath::Min( TMath::Min( TUINT( oSystemInfo.dwNumberOfProcessors ), MAX_THREADS ), a_rJob.uiNumPermutations );
	HANDLE      ahThreads[ MAX_THREADS ];
	TUINT       uiNumStarted = 0;

	for ( TUINT i = 1; i < uiNumThreads; i++ )
	{
		HANDLE hThread = CreateThread( TNULL, 0, CompileThreadProc, &a_rJob, 0, TNULL );
		if ( hThread )
			ahThreads[ uiNumStarted++ ] = hThread;
	}

	CompileThreadProc( &a_rJob );

	if ( uiNumStarted != 0 )
		WaitForMultipleObjects( uiNumStarted, ahThreads, TRUE, INFINITE );

	for ( TUINT i = 0; i < uiNumStarted; i++ )
		CloseHandle( ahThreads[ i ] );
}

//-----------------------------------------------------------------------------
// Bytecode cache: the unique shaders, the permutation table and the hash of every source file
// the compilation read. It's reused while the files, the combos and the entry point are the same
//-----------------------------------------------------------------------------
static constexpr TUINT32 SHADER_CACHE_MAGIC   = TFourCC( "TSCC" );
static constexpr TUINT32 SHADER_CACHE_VERSION = 1;

struct TShaderCacheHeader
{
	TUINT32 uiMagic;
	TUINT32 uiVersion;
	TUINT64 uiKey;
	TUINT32 uiNumFiles;
	TUINT32 uiNumPermutations;
	TUINT32 uiNumUniqueShaders;
};

static TUINT64 GetCacheKey( LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const TShaderCombo::Definition* a_pCombos, TUINT a_uiNumCombos )
{
	const UINT uiFlags = SHADER_COMPILE_FLAGS;

	TUINT64 uiKey = HashString( a_szEntryPoint, 14695981039346656037ull );
	uiKey         = HashString( a_szTarget, uiKey );
	uiKey         = HashData( &uiFlags, sizeof( uiFlags ), uiKey );
	uiKey         = HashString( TRenderD3DInterface::GetCommonShaderSource(), uiKey );

	for ( TUINT i = 0; i < a_uiNumCombos; i++ )
	{
		uiKey = HashString( a_pCombos[ i ].szName, uiKey );
		uiKey = HashData( &a_pCombos[ i ].iMinValue, sizeof( a_pCombos[ i ].iMinValue ), uiKey );
		uiKey = HashData( &a_pCombos[ i ].iMaxValue, sizeof( a_pCombos[ i ].iMaxValue ), uiKey );
		uiKey = HashData( &a_pCombos[ i ].eStages, sizeof( a_pCombos[ i ].eStages ), uiKey );
	}

	return uiKey;
}

class TCacheReader
{
public:
	TCacheReader( const TCHAR* a_szFileName )
	    : m_pData( TNULL ), m_uiSize( 0 ), m_uiOffset( 0 )
	{
		HANDLE hFile = CreateFileA( a_szFileName, GENERIC_READ, FILE_SHARE_READ, TNULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, TNULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			return;

		const DWORD uiSize = GetFileSize( hFile, TNULL );
		m_pData            = new TUINT8[ uiSize ];
		DWORD uiRead       = 0;

		if ( ReadFile( hFile, m_pData, uiSize, &uiRead, TNULL ) && uiRead == uiSize )
			m_uiSize = uiSize;

		CloseHandle( hFile );
	}

	~TCacheReader() { delete[] m_pData; }

	const void* ReadBytes( TUINT a_uiSize )
	{
		if ( m_uiOffset + a_uiSize > m_uiSize )
			return TNULL;

		const void* pData = m_pData + m_uiOffset;
		m_uiOffset += a_uiSize;
		return pData;
	}

	template <class T>
	TBOOL Read( T& a_rValue )
	{
		const void* pData = ReadBytes( sizeof( T ) );
		if ( pData ) TUtil::MemCopy( &a_rValue, pData, sizeof( T ) );
		return pData != TNULL;
	}

private:
	TUINT8* m_pData;
	TUINT   m_uiSize;
	TUINT   m_uiOffset;
};

TShaderCombo::TShaderCombo()
    : m_eStage( STAGE_VS ), m_uiNumPermutations( 0 ), m_uiNumUniqueShaders( 0 ), m_pPermutationToShader( TNULL ), m_ppBlobs( TNULL ), m_ppShaders( TNULL )
{
}

TShaderCombo::~TShaderCombo()
{
	Destroy();
}

TUINT TShaderCombo::GetNumPermutations( const Definition* a_pCombos, TUINT a_uiNumCombos )
{
	TUINT uiNumPermutations = 1;

	for ( TUINT i = 0; i < a_uiNumCombos; i++ )
		uiNumPermutations *= TUINT( a_pCombos[ i ].iMaxValue - a_pCombos[ i ].iMinValue + 1 );

	return uiNumPermutations;
}

TBOOL TShaderCombo::Create( const TCHAR* a_szFileName, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const Definition* a_pCombos, TUINT a_uiNumCombos, const TCHAR* a_szCacheFileName )
{
	Destroy();

	TShaderSourceCache              oCache;
	const TShaderSourceFile* pSource = oCache.Get( a_szFileName );

	if ( !pSource )
	{
		TERROR( "TShaderCombo: couldn't read '%s'\n", a_szFileName );
		return TFALSE;
	}

	if ( !ValidateCombos( a_szFileName, pSource->pchData, a_pCombos, a_uiNumCombos ) )
	{
		TASSERT( TFALSE && "The combos of the shader don't match its generated header" );
		return TFALSE;
	}

	m_eStage                      = ( a_szTarget[ 0 ] == 'v' ) ? STAGE_VS : ( a_szTarget[ 0 ] == 'c' ) ? STAGE_CS : STAGE_PS;
	const TUINT uiNumPermutations = GetNumPermutations( a_pCombos, a_uiNumCombos );
	const TUINT64 uiCacheKey      = GetCacheKey( a_szEntryPoint, a_szTarget, a_pCombos, a_uiNumCombos );

	m_pPermutationToShader = new TUINT16[ uiNumPermutations ];
	m_ppBlobs              = new ID3DBlob*[ uiNumPermutations ];
	m_ppShaders            = new ID3D11DeviceChild*[ uiNumPermutations ];
	TUtil::MemClear( m_ppShaders, uiNumPermutations * sizeof( *m_ppShaders ) );

	if ( !a_szCacheFileName || !LoadCache( a_szCacheFileName, uiCacheKey, uiNumPermutations, &oCache ) )
	{
		if ( !Compile( oCache, a_szFileName, *pSource, a_szEntryPoint, a_szTarget, a_pCombos, a_uiNumCombos, uiNumPermutations ) )
		{
			Destroy();
			return TFALSE;
		}

		if ( a_szCacheFileName )
			SaveCache( a_szCacheFileName, uiCacheKey, oCache );
	}

	for ( TUINT i = 0; i < m_uiNumUniqueShaders; i++ )
	{
		ID3DBlob* pBlob = m_ppBlobs[ i ];
		TBOOL     bCreated;

		if ( m_eStage == STAGE_VS )
			bCreated = TRenderD3DInterface::CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), TREINTERPRETCAST( ID3D11VertexShader**, &m_ppShaders[ i ] ) );
		else if ( m_eStage == STAGE_CS )
			bCreated = TRenderD3DInterface::CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), TREINTERPRETCAST( ID3D11ComputeShader**, &m_ppShaders[ i ] ) );
		else
			bCreated = TRenderD3DInterface::CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), TREINTERPRETCAST( ID3D11PixelShader**, &m_ppShaders[ i ] ) );

		if ( !bCreated )
		{
			Destroy();
			return TFALSE;
		}

#ifdef TOSHI_DEBUG
		const TCHAR* pchFileName = TMath::Max( strrchr( a_szFileName, '\\' ), strrchr( a_szFileName, '/' ) );

		TCHAR szName[ 128 ];
		T2String8::Format( szName, TINT( sizeof( szName ) ), "%.64s:%.32s#%u", pchFileName ? pchFileName + 1 : a_szFileName, a_szEntryPoint, i );
		m_ppShaders[ i ]->SetPrivateData( WKPDID_D3DDebugObjectName, T2String8::Length( szName ), szName );
#endif

		// Only the bytecode of vertex shaders is needed later, for the input layouts
		if ( m_eStage != STAGE_VS )
		{
			pBlob->Release();
			m_ppBlobs[ i ] = TNULL;
		}
	}

	return TTRUE;
}

TBOOL TShaderCombo::Compile( TShaderSourceCache& a_rCache, const TCHAR* a_szFileName, const TShaderSourceFile& a_rcSource, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const Definition* a_pCombos, TUINT a_uiNumCombos, TUINT a_uiNumPermutations )
{
	// Permutations that only differ in combos of the other stage share the shader of the
	// one with those combos at their minimum, which comes first
	TUINT* pCanonical    = new TUINT[ a_uiNumPermutations ];
	TUINT* pToCompile    = new TUINT[ a_uiNumPermutations ];
	TUINT  uiNumToCompile = 0;

	for ( TUINT i = 0; i < a_uiNumPermutations; i++ )
	{
		TUINT uiCanonical = i;

		for ( TUINT k = 0; k < a_uiNumCombos; k++ )
		{
			if ( !( a_pCombos[ k ].eStages & m_eStage ) )
				uiCanonical -= ( ( i / a_pCombos[ k ].uiStride ) % TUINT( a_pCombos[ k ].iMaxValue - a_pCombos[ k ].iMinValue + 1 ) ) * a_pCombos[ k ].uiStride;
		}

		pCanonical[ i ] = uiCanonical;

		if ( uiCanonical == i )
			pToCompile[ uiNumToCompile++ ] = i;
	}

	ID3DBlob** ppCompiled = new ID3DBlob*[ uiNumToCompile ];
	TUtil::MemClear( ppCompiled, uiNumToCompile * sizeof( *ppCompiled ) );

	TCompileJob oJob;
	oJob.pCache            = &a_rCache;
	oJob.szFileName        = a_szFileName;
	oJob.pSource           = &a_rcSource;
	oJob.szEntryPoint      = a_szEntryPoint;
	oJob.szTarget          = a_szTarget;
	oJob.pCombos           = a_pCombos;
	oJob.uiNumCombos       = a_uiNumCombos;
	oJob.pPermutations     = pToCompile;
	oJob.ppBlobs           = ppCompiled;
	oJob.uiNumPermutations = uiNumToCompile;
	oJob.iNext             = 0;
	oJob.bFailed           = FALSE;
	RunCompileJob( oJob );

	const TBOOL bCompiled = !oJob.bFailed;

	if ( bCompiled )
	{
		// Combos that don't change the code of this stage give the same bytecode
		ID3DBlob** ppStripped = new ID3DBlob*[ uiNumToCompile ];
		TUINT      uiCompiled = 0;

		for ( TUINT i = 0; i < a_uiNumPermutations; i++ )
		{
			if ( pCanonical[ i ] != i )
			{
				m_pPermutationToShader[ i ] = m_pPermutationToShader[ pCanonical[ i ] ];
				continue;
			}

			ID3DBlob* pBlob     = ppCompiled[ uiCompiled ];
			ID3DBlob* pStripped = StripBytecode( pBlob );
			TUINT     uiShader  = m_uiNumUniqueShaders;
			ppCompiled[ uiCompiled++ ] = TNULL;

			for ( TUINT k = 0; k < m_uiNumUniqueShaders; k++ )
			{
				if ( IsSameBytecode( ppStripped[ k ], pStripped ) )
				{
					uiShader = k;
					break;
				}
			}

			if ( uiShader == m_uiNumUniqueShaders )
			{
				m_ppBlobs[ m_uiNumUniqueShaders ]      = pBlob;
				ppStripped[ m_uiNumUniqueShaders++ ] = pStripped;
			}
			else
			{
				pBlob->Release();
				pStripped->Release();
			}

			TASSERT( uiShader <= TUINT16( -1 ) );
			m_pPermutationToShader[ i ] = TUINT16( uiShader );
		}

		for ( TUINT i = 0; i < m_uiNumUniqueShaders; i++ )
			ppStripped[ i ]->Release();

		delete[] ppStripped;
		m_uiNumPermutations = a_uiNumPermutations;
	}

	for ( TUINT i = 0; i < uiNumToCompile; i++ )
	{
		if ( ppCompiled[ i ] )
			ppCompiled[ i ]->Release();
	}

	delete[] ppCompiled;
	delete[] pToCompile;
	delete[] pCanonical;

	return bCompiled;
}

TBOOL TShaderCombo::LoadCache( const TCHAR* a_szCacheFileName, TUINT64 a_uiKey, TUINT a_uiNumPermutations, TShaderSourceCache* a_pSourceCache )
{
	TCacheReader       oReader( a_szCacheFileName );
	TShaderCacheHeader oHeader;

	if ( !oReader.Read( oHeader ) || oHeader.uiMagic != SHADER_CACHE_MAGIC || oHeader.uiVersion != SHADER_CACHE_VERSION ||
	     oHeader.uiKey != a_uiKey || oHeader.uiNumPermutations != a_uiNumPermutations || oHeader.uiNumUniqueShaders > a_uiNumPermutations )
		return TFALSE;

	// Every file the compilation read, the main file first
	for ( TUINT i = 0; i < oHeader.uiNumFiles; i++ )
	{
		TUINT32 uiNameLength;
		TUINT64 uiHash;

		const TCHAR* pchName = oReader.Read( uiNameLength ) ? TSTATICCAST( const TCHAR, oReader.ReadBytes( uiNameLength ) ) : TNULL;
		if ( !pchName || uiNameLength == 0 || pchName[ uiNameLength - 1 ] != '\0' || !oReader.Read( uiHash ) )
			return TFALSE;

		const TShaderSourceFile* pFile = a_pSourceCache->Get( pchName );
		if ( !pFile || pFile->uiHash != uiHash )
			return TFALSE;
	}

	const void* pTable = oReader.ReadBytes( a_uiNumPermutations * sizeof( TUINT16 ) );
	if ( !pTable )
		return TFALSE;

	TUtil::MemCopy( m_pPermutationToShader, pTable, a_uiNumPermutations * sizeof( TUINT16 ) );

	for ( TUINT i = 0; i < oHeader.uiNumUniqueShaders; i++ )
	{
		TUINT32     uiSize;
		const void* pBytecode = oReader.Read( uiSize ) ? oReader.ReadBytes( uiSize ) : TNULL;

		if ( !pBytecode || FAILED( D3DCreateBlob( uiSize, &m_ppBlobs[ m_uiNumUniqueShaders ] ) ) )
		{
			ReleaseBlobs();
			return TFALSE;
		}

		TUtil::MemCopy( m_ppBlobs[ m_uiNumUniqueShaders++ ]->GetBufferPointer(), pBytecode, uiSize );
	}

	for ( TUINT i = 0; i < a_uiNumPermutations; i++ )
	{
		if ( m_pPermutationToShader[ i ] >= m_uiNumUniqueShaders )
		{
			ReleaseBlobs();
			return TFALSE;
		}
	}

	m_uiNumPermutations = a_uiNumPermutations;
	return TTRUE;
}

void TShaderCombo::SaveCache( const TCHAR* a_szCacheFileName, TUINT64 a_uiKey, const TShaderSourceCache& a_rcSourceCache ) const
{
	HANDLE hFile = CreateFileA( a_szCacheFileName, GENERIC_WRITE, 0, TNULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, TNULL );

	if ( hFile == INVALID_HANDLE_VALUE )
	{
		TWARN( "TShaderCombo: couldn't write the cache '%s'\n", a_szCacheFileName );
		return;
	}

	auto fnWrite = [ hFile ]( const void* a_pData, TUINT a_uiSize ) {
		DWORD uiWritten;
		WriteFile( hFile, a_pData, a_uiSize, &uiWritten, TNULL );
	};

	const TShaderCacheHeader oHeader = { SHADER_CACHE_MAGIC, SHADER_CACHE_VERSION, a_uiKey, a_rcSourceCache.GetNumFiles(), m_uiNumPermutations, m_uiNumUniqueShaders };
	fnWrite( &oHeader, sizeof( oHeader ) );

	for ( TUINT i = 0; i < a_rcSourceCache.GetNumFiles(); i++ )
	{
		const TShaderSourceFile& rcFile       = a_rcSourceCache.GetFile( i );
		const TUINT32                   uiNameLength = rcFile.strFileName.Length() + 1;

		fnWrite( &uiNameLength, sizeof( uiNameLength ) );
		fnWrite( rcFile.strFileName.GetString(), uiNameLength );
		fnWrite( &rcFile.uiHash, sizeof( rcFile.uiHash ) );
	}

	fnWrite( m_pPermutationToShader, m_uiNumPermutations * sizeof( TUINT16 ) );

	for ( TUINT i = 0; i < m_uiNumUniqueShaders; i++ )
	{
		const TUINT32 uiSize = TUINT32( m_ppBlobs[ i ]->GetBufferSize() );
		fnWrite( &uiSize, sizeof( uiSize ) );
		fnWrite( m_ppBlobs[ i ]->GetBufferPointer(), uiSize );
	}

	CloseHandle( hFile );
}

void TShaderCombo::ReleaseBlobs()
{
	for ( TUINT i = 0; i < m_uiNumUniqueShaders; i++ )
	{
		if ( m_ppBlobs[ i ] )
			m_ppBlobs[ i ]->Release();
	}

	m_uiNumUniqueShaders = 0;
}

void TShaderCombo::Destroy()
{
	for ( TUINT i = 0; i < m_uiNumUniqueShaders; i++ )
	{
		if ( m_ppShaders && m_ppShaders[ i ] )
			m_ppShaders[ i ]->Release();

		if ( m_ppBlobs[ i ] )
			m_ppBlobs[ i ]->Release();
	}

	delete[] m_pPermutationToShader;
	delete[] m_ppBlobs;
	delete[] m_ppShaders;

	m_pPermutationToShader = TNULL;
	m_ppBlobs              = TNULL;
	m_ppShaders            = TNULL;
	m_uiNumPermutations    = 0;
	m_uiNumUniqueShaders   = 0;
}

ID3DBlob* TShaderCombo::GetBlob( TUINT a_uiPermutation ) const
{
	TASSERT( m_eStage == STAGE_VS && a_uiPermutation < m_uiNumPermutations );
	return m_ppBlobs[ m_pPermutationToShader[ a_uiPermutation ] ];
}

ID3D11VertexShader* TShaderCombo::GetVertexShader( TUINT a_uiPermutation ) const
{
	TASSERT( m_eStage == STAGE_VS && a_uiPermutation < m_uiNumPermutations );
	return TSTATICCAST( ID3D11VertexShader, m_ppShaders[ m_pPermutationToShader[ a_uiPermutation ] ] );
}

ID3D11PixelShader* TShaderCombo::GetPixelShader( TUINT a_uiPermutation ) const
{
	TASSERT( m_eStage == STAGE_PS && a_uiPermutation < m_uiNumPermutations );
	return TSTATICCAST( ID3D11PixelShader, m_ppShaders[ m_pPermutationToShader[ a_uiPermutation ] ] );
}

ID3D11ComputeShader* TShaderCombo::GetComputeShader( TUINT a_uiPermutation ) const
{
	TASSERT( m_eStage == STAGE_CS && a_uiPermutation < m_uiNumPermutations );
	return TSTATICCAST( ID3D11ComputeShader, m_ppShaders[ m_pPermutationToShader[ a_uiPermutation ] ] );
}

TOSHI_NAMESPACE_END
