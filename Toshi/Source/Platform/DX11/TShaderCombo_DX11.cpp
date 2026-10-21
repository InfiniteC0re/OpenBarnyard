#include "ToshiPCH.h"
#include "TShaderCombo_DX11.h"
#include "TRenderInterface_DX11.h"

#include "File/TFile.h"

#include <d3dcompiler.h>
#include <d3dcommon.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// The source files read while compiling one entry point, every permutation includes the same ones
class TShaderSourceCache
{
public:
	~TShaderSourceCache()
	{
		for ( TUINT i = 0; i < m_uiNumFiles; i++ )
			delete[] m_aFiles[ i ].pchData;
	}

	TBOOL Get( const TString8& a_rcFileName, const TCHAR*& a_rpchData, TUINT& a_ruiSize )
	{
		for ( TUINT i = 0; i < m_uiNumFiles; i++ )
		{
			if ( m_aFiles[ i ].strFileName.CompareNoCase( a_rcFileName.GetString() ) == 0 )
			{
				a_rpchData = m_aFiles[ i ].pchData;
				a_ruiSize  = m_aFiles[ i ].uiSize;
				return TTRUE;
			}
		}

		if ( m_uiNumFiles == MAX_FILES )
			return TFALSE;

		TFile* pFile = TFile::Create( a_rcFileName );
		if ( !pFile )
			return TFALSE;

		const TUINT uiSize  = TUINT( pFile->GetSize() );
		TCHAR*      pchData = new TCHAR[ uiSize + 1 ];
		const TBOOL bRead   = pFile->Read( pchData, uiSize ) == uiSize;
		pFile->Destroy();

		if ( !bRead )
		{
			delete[] pchData;
			return TFALSE;
		}

		pchData[ uiSize ] = '\0';

		File& rFile      = m_aFiles[ m_uiNumFiles++ ];
		rFile.strFileName = a_rcFileName;
		rFile.pchData     = pchData;
		rFile.uiSize      = uiSize;

		a_rpchData = pchData;
		a_ruiSize  = uiSize;
		return TTRUE;
	}

private:
	static constexpr TUINT MAX_FILES = 32;

	struct File
	{
		TString8 strFileName;
		TCHAR*   pchData;
		TUINT    uiSize;
	};

	File  m_aFiles[ MAX_FILES ];
	TUINT m_uiNumFiles = 0;
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

		TString8     strFileName = rcDirectory + a_szFileName;
		const TCHAR* pchData;
		TUINT        uiSize;

		if ( !m_rCache.Get( strFileName, pchData, uiSize ) )
		{
			strFileName = m_strRootDirectory + a_szFileName;

			if ( !m_rCache.Get( strFileName, pchData, uiSize ) )
				return E_FAIL;
		}

		*a_ppData   = pchData;
		*a_puiBytes = uiSize;
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

static ID3DBlob* CompilePermutation( TShaderSourceCache& a_rCache, const TCHAR* a_szFileName, const TCHAR* a_pchSource, TUINT a_uiSize, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const TShaderCombo::Definition* a_pCombos, TUINT a_uiNumCombos, TUINT a_uiPermutation )
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

	UINT uiFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#ifdef TOSHI_DEBUG
	uiFlags |= D3DCOMPILE_DEBUG;
#endif

	TShaderIncludeHandler oIncludeHandler( a_rCache, a_szFileName );
	ID3DBlob*             pBlob      = TNULL;
	ID3DBlob*             pErrorBlob = TNULL;

	if ( FAILED( D3DCompile( a_pchSource, a_uiSize, a_szFileName, aDefines, &oIncludeHandler, a_szEntryPoint, a_szTarget, uiFlags, 0, &pBlob, &pErrorBlob ) ) )
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

// The stripped bytecode of the unique shaders while they're compiled
struct TStrippedBlobs
{
	TStrippedBlobs( TUINT a_uiMaxBlobs )
	    : ppBlobs( new ID3DBlob*[ a_uiMaxBlobs ] ), uiNumBlobs( 0 )
	{
	}

	~TStrippedBlobs()
	{
		for ( TUINT i = 0; i < uiNumBlobs; i++ )
			ppBlobs[ i ]->Release();

		delete[] ppBlobs;
	}

	ID3DBlob** ppBlobs;
	TUINT      uiNumBlobs;
};

static TBOOL IsSameBytecode( ID3DBlob* a_pBlob1, ID3DBlob* a_pBlob2 )
{
	return a_pBlob1->GetBufferSize() == a_pBlob2->GetBufferSize() &&
	    TUtil::MemCompare( a_pBlob1->GetBufferPointer(), a_pBlob2->GetBufferPointer(), a_pBlob1->GetBufferSize() ) == 0;
}

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

TBOOL TShaderCombo::Create( const TCHAR* a_szFileName, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const Definition* a_pCombos, TUINT a_uiNumCombos )
{
	Destroy();

	TShaderSourceCache oCache;
	const TCHAR*       pchSource;
	TUINT              uiSize;

	if ( !oCache.Get( a_szFileName, pchSource, uiSize ) )
	{
		TERROR( "TShaderCombo: couldn't read '%s'\n", a_szFileName );
		return TFALSE;
	}

	if ( !ValidateCombos( a_szFileName, pchSource, a_pCombos, a_uiNumCombos ) )
	{
		TASSERT( TFALSE && "The combos of the shader don't match its generated header" );
		return TFALSE;
	}

	m_eStage                          = ( a_szTarget[ 0 ] == 'v' ) ? STAGE_VS : ( a_szTarget[ 0 ] == 'c' ) ? STAGE_CS : STAGE_PS;
	const TUINT uiNumPermutations     = GetNumPermutations( a_pCombos, a_uiNumCombos );
	m_pPermutationToShader            = new TUINT16[ uiNumPermutations ];
	m_ppBlobs                         = new ID3DBlob*[ uiNumPermutations ];
	m_ppShaders                       = new ID3D11DeviceChild*[ uiNumPermutations ];
	TUtil::MemClear( m_ppShaders, uiNumPermutations * sizeof( *m_ppShaders ) );

	TStrippedBlobs oStripped( uiNumPermutations );

	for ( TUINT i = 0; i < uiNumPermutations; i++ )
	{
		// Permutations that only differ in combos of the other stage share the shader of the
		// one with those combos at their minimum, which comes first
		TUINT uiCanonical = i;

		for ( TUINT k = 0; k < a_uiNumCombos; k++ )
		{
			if ( !( a_pCombos[ k ].eStages & m_eStage ) )
				uiCanonical -= ( ( i / a_pCombos[ k ].uiStride ) % TUINT( a_pCombos[ k ].iMaxValue - a_pCombos[ k ].iMinValue + 1 ) ) * a_pCombos[ k ].uiStride;
		}

		if ( uiCanonical != i )
		{
			m_pPermutationToShader[ i ] = m_pPermutationToShader[ uiCanonical ];
			continue;
		}

		ID3DBlob* pBlob = CompilePermutation( oCache, a_szFileName, pchSource, uiSize, a_szEntryPoint, a_szTarget, a_pCombos, a_uiNumCombos, i );

		if ( !pBlob )
		{
			Destroy();
			return TFALSE;
		}

		// Combos that don't change the code of this stage give the same bytecode
		ID3DBlob* pStripped = StripBytecode( pBlob );
		TUINT     uiShader  = m_uiNumUniqueShaders;

		for ( TUINT k = 0; k < m_uiNumUniqueShaders; k++ )
		{
			if ( IsSameBytecode( oStripped.ppBlobs[ k ], pStripped ) )
			{
				uiShader = k;
				break;
			}
		}

		if ( uiShader == m_uiNumUniqueShaders )
		{
			m_ppBlobs[ m_uiNumUniqueShaders++ ]            = pBlob;
			oStripped.ppBlobs[ oStripped.uiNumBlobs++ ] = pStripped;
		}
		else
		{
			pBlob->Release();
			pStripped->Release();
		}

		TASSERT( uiShader <= TUINT16( -1 ) );
		m_pPermutationToShader[ i ] = TUINT16( uiShader );
	}

	m_uiNumPermutations = uiNumPermutations;

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
		TCHAR szName[ 128 ];
		T2String8::Format( szName, TINT( sizeof( szName ) ), "%s:%s#%u", a_szFileName, a_szEntryPoint, i );
		m_ppShaders[ i ]->SetPrivateData( WKPDID_D3DDebugObjectName, T2String8::Length( szName ), szName );
#endif
	}

	return TTRUE;
}

void TShaderCombo::Destroy()
{
	for ( TUINT i = 0; i < m_uiNumUniqueShaders; i++ )
	{
		if ( m_ppShaders[ i ] )
			m_ppShaders[ i ]->Release();

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
	TASSERT( a_uiPermutation < m_uiNumPermutations );
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
