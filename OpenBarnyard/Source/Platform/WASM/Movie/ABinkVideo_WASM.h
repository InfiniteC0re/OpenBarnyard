#pragma once

#include "ABinkBits_WASM.h"

// Bink 1 video, revisions c to k. Every frame is decoded into the older of two sets of planes.
class ABinkVideo
{
public:
	enum PLANE_
	{
		PLANE_Y,
		PLANE_U,
		PLANE_V,
		PLANE_ALPHA,
		PLANE_NUMOF
	};

	static TUINT32 GetMemorySize( TINT a_iWidth, TINT a_iHeight, TUINT32 a_uiFlags );

	// a_pMemory is GetMemorySize() bytes
	void  Create( void* a_pMemory, TINT a_iWidth, TINT a_iHeight, TCHAR a_cRevision, TUINT32 a_uiFlags );
	TBOOL Decode( const TUINT8* a_pData, TUINT32 a_uiSize );

	// Returns the number of rows written
	TINT CopyToSurface( TUINT8* a_pDst, TINT a_iPitch, TUINT32 a_uiHeight, TUINT32 a_uiX, TUINT32 a_uiY, TUINT32 a_uiFormat ) const;

	const TUINT8* GetPlane( TINT a_iPlane ) const { return m_apPlanes[ a_iPlane ]; }
	TINT          GetStride( TINT a_iPlane ) const { return m_aiStrides[ a_iPlane ]; }

private:
	enum BUNDLE_
	{
		BUNDLE_BLOCK_TYPES,
		BUNDLE_SUB_BLOCK_TYPES,
		BUNDLE_COLORS,
		BUNDLE_PATTERN,
		BUNDLE_X_OFF,
		BUNDLE_Y_OFF,
		BUNDLE_INTRA_DC,
		BUNDLE_INTER_DC,
		BUNDLE_RUN,
		BUNDLE_NUMOF
	};

	struct Tree
	{
		TINT   iTable;
		TUINT8 aSymbols[ 16 ];
	};

	struct Bundle
	{
		TUINT8* pData;
		TINT    iLength;
		Tree    oTree;
		TINT    iCurDec; // -1: no more values in this plane
		TINT    iCurPtr;
	};

	void  InitLengths( TINT a_iWidth, TINT a_iBlocksWide );
	TBOOL ReadTree( Tree& a_rTree );
	TBOOL ReadBundleHeader( TINT a_iBundle );
	TINT  ReadSymbol( const Tree& a_rTree );
	TINT  ReadCount( Bundle& a_rBundle );
	void  ReadRuns( Bundle& a_rBundle );
	void  ReadMotionValues( Bundle& a_rBundle );
	TBOOL ReadBlockTypes( Bundle& a_rBundle );
	TBOOL ReadPatterns( Bundle& a_rBundle );
	TBOOL ReadColors( Bundle& a_rBundle );
	TBOOL ReadDCs( Bundle& a_rBundle, TINT a_iStartBits, TINT a_iHasSign );
	TINT  ReadDctCoeffs( TINT* a_pBlock );
	void  Unquantize( TINT* a_pBlock, const TINT* a_pQuant, TINT a_iQuant );
	void  ReadResidue( TINT* a_pBlock, TINT a_iMasksCount );
	TBOOL DecodePlane( TINT a_iPlane, TBOOL a_bChroma );

private:
	ABinkBits m_oBits;
	TINT      m_iWidth;
	TINT      m_iHeight;
	TINT      m_iVersion;
	TBOOL     m_bHasAlpha;
	TBOOL     m_bSwapChroma;
	TINT      m_aiStrides[ PLANE_NUMOF ];
	TINT      m_aiRows[ PLANE_NUMOF ];
	TUINT8*   m_apPlanes[ PLANE_NUMOF ];
	TUINT8*   m_apPrevious[ PLANE_NUMOF ];
	Bundle    m_aBundles[ BUNDLE_NUMOF ];
	Tree      m_aColorHigh[ 16 ];
	TINT      m_iColorLast;
	TINT      m_aiCoefIndices[ 64 ];
	TINT      m_iCoefCount;
};
