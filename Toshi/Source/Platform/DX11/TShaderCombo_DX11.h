#pragma once
#include <d3d11.h>

TOSHI_NAMESPACE_START

class TShaderSourceCache;
struct TShaderSourceFile;

//-----------------------------------------------------------------------------
// Purpose: Every permutation of one entry point of a .hlsl file.
// The combos are declared at the top of the file, before any code:
//   // STATIC: "NAME" "min..max" [vs|ps|cs]
// The optional stage tag says which stage the combo affects, untagged combos affect all of them.
// Each combo becomes a define with its value, e.g. "#if FOG".
// DevTools/Shaders/GenerateShaderCombos.py turns the declarations into a header with the
// definition table and a helper per combo, the index of a permutation is the sum of them:
//   oPixelShaders.GetPixelShader( GrassShaderCombos::FOG( bFog ) + GrassShaderCombos::TEXTURED( 1 ) )
// "TShaderCommon.hlsli" can be included for the shared helpers (AlphaTest).
// The permutations compile on all the cores. Create doesn't touch the render state, so it can
// run on a worker thread while the game renders.
//-----------------------------------------------------------------------------
class MODAPI TShaderCombo
{
public:
	using STAGE = TUINT;
	enum STAGE_ : STAGE
	{
		STAGE_VS  = BITFLAG( 0 ),
		STAGE_PS  = BITFLAG( 1 ),
		STAGE_CS  = BITFLAG( 2 ),
		STAGE_ALL = STAGE_VS | STAGE_PS | STAGE_CS,
	};

	struct Definition
	{
		const TCHAR* szName;
		TINT         iMinValue;
		TINT         iMaxValue;
		TUINT        uiStride;
		STAGE        eStages;
	};

	static constexpr const TCHAR* COMMON_INCLUDE_NAME = "TShaderCommon.hlsli";

public:
	TShaderCombo();
	~TShaderCombo();

	TShaderCombo( const TShaderCombo& )            = delete;
	TShaderCombo& operator=( const TShaderCombo& ) = delete;

	// Compiles and creates the shaders of every permutation, a_szTarget picks the stage (vs_*, ps_* or cs_*).
	// With a_szCacheFileName the bytecode is kept in that file and reused while the source files
	// (includes too), the combos and the entry point stay the same
	TBOOL Create( const TCHAR* a_szFileName, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const Definition* a_pCombos, TUINT a_uiNumCombos, const TCHAR* a_szCacheFileName = TNULL );

	// Same, with the definitions of a generated header
	template <class Combos>
	TBOOL Create( const TCHAR* a_szFileName, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const TCHAR* a_szCacheFileName = TNULL )
	{
		return Create( a_szFileName, a_szEntryPoint, a_szTarget, Combos::s_aCombos, Combos::NUM_COMBOS, a_szCacheFileName );
	}

	void Destroy();

	TBOOL IsCreated() const { return m_uiNumPermutations != 0; }
	TUINT GetNumPermutations() const { return m_uiNumPermutations; }
	TUINT GetNumUniqueShaders() const { return m_uiNumUniqueShaders; }

	// The bytecode of a vertex shader, for creating input layouts
	ID3DBlob* GetBlob( TUINT a_uiPermutation ) const;

	ID3D11VertexShader*  GetVertexShader( TUINT a_uiPermutation ) const;
	ID3D11PixelShader*   GetPixelShader( TUINT a_uiPermutation ) const;
	ID3D11ComputeShader* GetComputeShader( TUINT a_uiPermutation ) const;

	// Number of permutations of a definition table
	static TUINT GetNumPermutations( const Definition* a_pCombos, TUINT a_uiNumCombos );

private:
	TBOOL Compile( TShaderSourceCache& a_rCache, const TCHAR* a_szFileName, const TShaderSourceFile& a_rcSource, LPCSTR a_szEntryPoint, LPCSTR a_szTarget, const Definition* a_pCombos, TUINT a_uiNumCombos, TUINT a_uiNumPermutations );
	TBOOL LoadCache( const TCHAR* a_szCacheFileName, TUINT64 a_uiKey, TUINT a_uiNumPermutations, TShaderSourceCache* a_pSourceCache );
	void  SaveCache( const TCHAR* a_szCacheFileName, TUINT64 a_uiKey, const TShaderSourceCache& a_rcSourceCache ) const;
	void  ReleaseBlobs();

private:
	STAGE                m_eStage;
	TUINT                m_uiNumPermutations;
	TUINT                m_uiNumUniqueShaders;
	TUINT16*             m_pPermutationToShader; // index into m_ppBlobs/m_ppShaders
	ID3DBlob**           m_ppBlobs;
	ID3D11DeviceChild**  m_ppShaders;
};

TOSHI_NAMESPACE_END
