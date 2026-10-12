#pragma once
#include <Toshi/TObject.h>

// Bit array of the flags that the tracks of a scripted sequence (ASSeq) raise and wait for.
//
// TClass 0x007b4bf0 (size 0xc, runtime creatable), vtable 0x00739368.
class ASSeqFlag : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ASSeqFlag, Toshi::TObject );

public:
	// NOTE: CreateObject (0062b4d0) leaves the members uninitialised
	ASSeqFlag() = default;
	ASSeqFlag( TINT a_iNumFlags );
	~ASSeqFlag();

	// Lowers every flag
	void Clear();

	// $Barnyard: FUNCTION 0062b5e0
	static TINT GetWordIndex( TINT a_iFlag ) { return a_iFlag / 32; }

	// $Barnyard: FUNCTION 0062b5f0
	static TINT GetBitIndex( TINT a_iFlag ) { return a_iFlag % 32; }

	void  Set( TINT a_iFlag ) { m_puiFlags[ GetWordIndex( a_iFlag ) ] |= 1 << GetBitIndex( a_iFlag ); }
	TBOOL IsSet( TINT a_iFlag ) const { return ( m_puiFlags[ GetWordIndex( a_iFlag ) ] & ( 1 << GetBitIndex( a_iFlag ) ) ) != 0; }

private:
	TINT     m_iNumFlags; // 0x4
	TUINT32* m_puiFlags;  // 0x8, TMalloc'd
};

TSTATICASSERT( sizeof( ASSeqFlag ) == 0xc );
