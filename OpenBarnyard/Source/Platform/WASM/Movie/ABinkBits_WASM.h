#pragma once

#include <string.h>

// Bink reads the bits of each little endian word from the lowest one up, which on wasm is the same as reading the
// bytes in order. The buffer has to be padded: a peek may read a few bytes past the packet.
class ABinkBits
{
public:
	void Init( const TUINT8* a_pData, TUINT32 a_uiSize )
	{
		m_pData = a_pData;
		m_uiPos = 0;
		m_uiEnd = a_uiSize * 8;
	}

	TINT  GetLeft() const { return TINT( m_uiEnd ) - TINT( m_uiPos ); }
	TBOOL IsOver() const { return m_uiPos > m_uiEnd; }
	TBOOL IsAtEnd() const { return m_uiPos >= m_uiEnd; }

	// Up to 25 bits
	TUINT32 Peek( TINT a_iCount ) const
	{
		TUINT32 uiWord;
		memcpy( &uiWord, m_pData + ( m_uiPos >> 3 ), 4 );
		return ( uiWord >> ( m_uiPos & 7 ) ) & ( ( 1u << a_iCount ) - 1 );
	}

	TUINT32 Read( TINT a_iCount )
	{
		const TUINT32 uiValue = Peek( a_iCount );
		m_uiPos += a_iCount;
		return uiValue;
	}

	TUINT32 ReadBit()
	{
		const TUINT32 uiValue = ( m_pData[ m_uiPos >> 3 ] >> ( m_uiPos & 7 ) ) & 1;
		m_uiPos += 1;
		return uiValue;
	}

	void Skip( TINT a_iCount ) { m_uiPos += a_iCount; }
	void Align32() { m_uiPos = ( m_uiPos + 31 ) & ~31u; }

private:
	const TUINT8* m_pData;
	TUINT32       m_uiPos;
	TUINT32       m_uiEnd;
};
