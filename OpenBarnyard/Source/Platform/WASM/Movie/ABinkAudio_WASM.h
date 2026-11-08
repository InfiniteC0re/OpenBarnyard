#pragma once

#include "ABinkBits_WASM.h"

// Bink 1 audio tracks, both the DCT and the RDFT kind
class ABinkAudio
{
public:
	typedef void ( *t_fnOutput )( const TFLOAT* a_pSamples, TINT a_iChannels, TINT a_iFrames, void* a_pUser );

	static TUINT32 GetMemorySize( TINT a_iSampleRate, TUINT32 a_uiFlags );

	// a_pMemory is GetMemorySize() bytes
	void Create( void* a_pMemory, TINT a_iSampleRate, TUINT32 a_uiFlags );
	void Reset() { m_bFirst = TTRUE; }

	// Decodes one packet, the samples are in -1..1, interleaved
	void Decode( const TUINT8* a_pData, TUINT32 a_uiSize, t_fnOutput a_fnOutput, void* a_pUser );

	TINT GetChannels() const { return m_iOutChannels; }

private:
	TFLOAT GetFloat();
	TBOOL  DecodeBlock();
	void   InverseDct( TFLOAT* a_pOut );
	void   InverseRdft( TFLOAT* a_pOut );
	void   InverseRealFft( TFLOAT* a_pOut, TFLOAT a_fScale );
	void   InverseFft();

private:
	ABinkBits m_oBits;
	TBOOL     m_bUseDct;
	TBOOL     m_bFirst;
	TINT      m_iOutChannels;
	TINT      m_iChannels;
	TINT      m_iFrameLength;
	TINT      m_iOverlap;
	TINT      m_iBlockSamples;
	TINT      m_iNumBands;
	TFLOAT    m_fRoot;
	TFLOAT    m_afQuantTable[ 96 ];
	TFLOAT    m_afQuant[ 25 ];
	TINT      m_aiBands[ 26 ];

	TFLOAT*  m_pCoeffs;
	TFLOAT*  m_apOut[ 2 ];
	TFLOAT*  m_apPrevious[ 2 ];
	TFLOAT*  m_pInterleaved;
	TFLOAT*  m_pSpectrumRe; // half of a hermitian spectrum
	TFLOAT*  m_pSpectrumIm;
	TFLOAT*  m_pFftRe;
	TFLOAT*  m_pFftIm;
	TFLOAT*  m_pFftCos;
	TFLOAT*  m_pFftSin;
	TFLOAT*  m_pUnpackCos;
	TFLOAT*  m_pUnpackSin;
	TFLOAT*  m_pDctCos;
	TFLOAT*  m_pDctSin;
	TUINT16* m_pReverse;
};
