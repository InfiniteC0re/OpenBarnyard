#pragma once
#include "AUnitStaticData.h"

// Marks a unit type as an actor of the scripted sequence (SSeq) system.
class ASSeqActorData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( ASSeqActorData, AUnitStaticData );

#ifdef BARNYARD_LATEST
	// The Wii build adds the actors of its new sequences (SSCUOc, SSBFF, SS06 merged, SSUL)
	static constexpr TINT NUM_ACTORS = 0x11e;
#else
	static constexpr TINT NUM_ACTORS = 0x109;
#endif

public:
	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	TINT GetActorIndex() const { return m_iActorIndex; }

	// Index of a_szName in the SSeq actor table (case insensitive), -1 if unknown
	static TINT FindActorIndex( const TCHAR* a_szName );

	// $Barnyard: FUNCTION 00629ca0
	// $BarnyardWiiJP: FUNCTION 803da1e8
	static const TCHAR* GetActorName( TINT a_iIndex ) { return ms_aActorNames[ a_iIndex ]; }

public:
	TINT m_iActorIndex; // 0x0c

	static const TCHAR* ms_aActorNames[ NUM_ACTORS ];
};

TSTATICASSERT( sizeof( ASSeqActorData ) == 0x10 );
