#include "ToshiPCH.h"
#include "TBitArray.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

#define MIN_NUM_BITS    ( 8 * sizeof( TUINT32 ) )
#define BIT_SUBNUM_MASK ( MIN_NUM_BITS - 1 )

TBitArray::TBitArray( TINT a_iNumBits, TINT a_iInitialiseWith )
{
	Create( a_iNumBits, a_iInitialiseWith );
}

TBitArray::~TBitArray()
{
	Destroy();
}

TINT TBitArray::GetNumAllocatedBytes() const
{
	return sizeof( TUINT32 ) * ( ( m_iSize + BIT_SUBNUM_MASK ) / MIN_NUM_BITS );
}

TINT TBitArray::FindFirstClearBit()
{
	TUINT uiNumElements = GetNumAllocatedBytes() / sizeof( TUINT32 );

	// This is... um... amazing... in some way
	// I know I could do this with a for cycle or even macro but I just had some great time meditating while writing this
	// And yep, there's no for cycle in the original code too
	for ( TUINT i = 0; i < uiNumElements; i++ )
	{
		if ( ( m_pBuffer[ i ] & ( 1U << 0U ) ) == 0 )
			return MIN_NUM_BITS * i + 0;
		if ( ( m_pBuffer[ i ] & ( 1U << 1U ) ) == 0 )
			return MIN_NUM_BITS * i + 1;
		if ( ( m_pBuffer[ i ] & ( 1U << 2U ) ) == 0 )
			return MIN_NUM_BITS * i + 2;
		if ( ( m_pBuffer[ i ] & ( 1U << 3U ) ) == 0 )
			return MIN_NUM_BITS * i + 3;
		if ( ( m_pBuffer[ i ] & ( 1U << 4U ) ) == 0 )
			return MIN_NUM_BITS * i + 4;
		if ( ( m_pBuffer[ i ] & ( 1U << 5U ) ) == 0 )
			return MIN_NUM_BITS * i + 5;
		if ( ( m_pBuffer[ i ] & ( 1U << 6U ) ) == 0 )
			return MIN_NUM_BITS * i + 6;
		if ( ( m_pBuffer[ i ] & ( 1U << 7U ) ) == 0 )
			return MIN_NUM_BITS * i + 7;
		if ( ( m_pBuffer[ i ] & ( 1U << 8U ) ) == 0 )
			return MIN_NUM_BITS * i + 8;
		if ( ( m_pBuffer[ i ] & ( 1U << 9U ) ) == 0 )
			return MIN_NUM_BITS * i + 9;
		if ( ( m_pBuffer[ i ] & ( 1U << 10U ) ) == 0 )
			return MIN_NUM_BITS * i + 10;
		if ( ( m_pBuffer[ i ] & ( 1U << 11U ) ) == 0 )
			return MIN_NUM_BITS * i + 11;
		if ( ( m_pBuffer[ i ] & ( 1U << 12U ) ) == 0 )
			return MIN_NUM_BITS * i + 12;
		if ( ( m_pBuffer[ i ] & ( 1U << 13U ) ) == 0 )
			return MIN_NUM_BITS * i + 13;
		if ( ( m_pBuffer[ i ] & ( 1U << 14U ) ) == 0 )
			return MIN_NUM_BITS * i + 14;
		if ( ( m_pBuffer[ i ] & ( 1U << 15U ) ) == 0 )
			return MIN_NUM_BITS * i + 15;
		if ( ( m_pBuffer[ i ] & ( 1U << 16U ) ) == 0 )
			return MIN_NUM_BITS * i + 16;
		if ( ( m_pBuffer[ i ] & ( 1U << 17U ) ) == 0 )
			return MIN_NUM_BITS * i + 17;
		if ( ( m_pBuffer[ i ] & ( 1U << 18U ) ) == 0 )
			return MIN_NUM_BITS * i + 18;
		if ( ( m_pBuffer[ i ] & ( 1U << 19U ) ) == 0 )
			return MIN_NUM_BITS * i + 19;
		if ( ( m_pBuffer[ i ] & ( 1U << 20U ) ) == 0 )
			return MIN_NUM_BITS * i + 20;
		if ( ( m_pBuffer[ i ] & ( 1U << 21U ) ) == 0 )
			return MIN_NUM_BITS * i + 21;
		if ( ( m_pBuffer[ i ] & ( 1U << 22U ) ) == 0 )
			return MIN_NUM_BITS * i + 22;
		if ( ( m_pBuffer[ i ] & ( 1U << 23U ) ) == 0 )
			return MIN_NUM_BITS * i + 23;
		if ( ( m_pBuffer[ i ] & ( 1U << 24U ) ) == 0 )
			return MIN_NUM_BITS * i + 24;
		if ( ( m_pBuffer[ i ] & ( 1U << 25U ) ) == 0 )
			return MIN_NUM_BITS * i + 25;
		if ( ( m_pBuffer[ i ] & ( 1U << 26U ) ) == 0 )
			return MIN_NUM_BITS * i + 26;
		if ( ( m_pBuffer[ i ] & ( 1U << 27U ) ) == 0 )
			return MIN_NUM_BITS * i + 27;
		if ( ( m_pBuffer[ i ] & ( 1U << 28U ) ) == 0 )
			return MIN_NUM_BITS * i + 28;
		if ( ( m_pBuffer[ i ] & ( 1U << 29U ) ) == 0 )
			return MIN_NUM_BITS * i + 29;
		if ( ( m_pBuffer[ i ] & ( 1U << 30U ) ) == 0 )
			return MIN_NUM_BITS * i + 30;
		if ( ( m_pBuffer[ i ] & ( 1U << 31U ) ) == 0 )
			return MIN_NUM_BITS * i + 31;
	}

	return -1;
}

TINT TBitArray::FindFirstSetBit()
{
	TUINT uiNumElements = GetNumAllocatedBytes() / sizeof( TUINT32 );

	// This is... um... amazing... in some way
	// I know I could do this with a for cycle or even macro but I just had some great time meditating while writing this
	// And yep, there's no for cycle in the original code too
	for ( TUINT i = 0; i < uiNumElements; i++ )
	{
		if ( ( m_pBuffer[ i ] & ( 1U << 0U ) ) != 0 )
			return MIN_NUM_BITS * i + 0;
		if ( ( m_pBuffer[ i ] & ( 1U << 1U ) ) != 0 )
			return MIN_NUM_BITS * i + 1;
		if ( ( m_pBuffer[ i ] & ( 1U << 2U ) ) != 0 )
			return MIN_NUM_BITS * i + 2;
		if ( ( m_pBuffer[ i ] & ( 1U << 3U ) ) != 0 )
			return MIN_NUM_BITS * i + 3;
		if ( ( m_pBuffer[ i ] & ( 1U << 4U ) ) != 0 )
			return MIN_NUM_BITS * i + 4;
		if ( ( m_pBuffer[ i ] & ( 1U << 5U ) ) != 0 )
			return MIN_NUM_BITS * i + 5;
		if ( ( m_pBuffer[ i ] & ( 1U << 6U ) ) != 0 )
			return MIN_NUM_BITS * i + 6;
		if ( ( m_pBuffer[ i ] & ( 1U << 7U ) ) != 0 )
			return MIN_NUM_BITS * i + 7;
		if ( ( m_pBuffer[ i ] & ( 1U << 8U ) ) != 0 )
			return MIN_NUM_BITS * i + 8;
		if ( ( m_pBuffer[ i ] & ( 1U << 9U ) ) != 0 )
			return MIN_NUM_BITS * i + 9;
		if ( ( m_pBuffer[ i ] & ( 1U << 10U ) ) != 0 )
			return MIN_NUM_BITS * i + 10;
		if ( ( m_pBuffer[ i ] & ( 1U << 11U ) ) != 0 )
			return MIN_NUM_BITS * i + 11;
		if ( ( m_pBuffer[ i ] & ( 1U << 12U ) ) != 0 )
			return MIN_NUM_BITS * i + 12;
		if ( ( m_pBuffer[ i ] & ( 1U << 13U ) ) != 0 )
			return MIN_NUM_BITS * i + 13;
		if ( ( m_pBuffer[ i ] & ( 1U << 14U ) ) != 0 )
			return MIN_NUM_BITS * i + 14;
		if ( ( m_pBuffer[ i ] & ( 1U << 15U ) ) != 0 )
			return MIN_NUM_BITS * i + 15;
		if ( ( m_pBuffer[ i ] & ( 1U << 16U ) ) != 0 )
			return MIN_NUM_BITS * i + 16;
		if ( ( m_pBuffer[ i ] & ( 1U << 17U ) ) != 0 )
			return MIN_NUM_BITS * i + 17;
		if ( ( m_pBuffer[ i ] & ( 1U << 18U ) ) != 0 )
			return MIN_NUM_BITS * i + 18;
		if ( ( m_pBuffer[ i ] & ( 1U << 19U ) ) != 0 )
			return MIN_NUM_BITS * i + 19;
		if ( ( m_pBuffer[ i ] & ( 1U << 20U ) ) != 0 )
			return MIN_NUM_BITS * i + 20;
		if ( ( m_pBuffer[ i ] & ( 1U << 21U ) ) != 0 )
			return MIN_NUM_BITS * i + 21;
		if ( ( m_pBuffer[ i ] & ( 1U << 22U ) ) != 0 )
			return MIN_NUM_BITS * i + 22;
		if ( ( m_pBuffer[ i ] & ( 1U << 23U ) ) != 0 )
			return MIN_NUM_BITS * i + 23;
		if ( ( m_pBuffer[ i ] & ( 1U << 24U ) ) != 0 )
			return MIN_NUM_BITS * i + 24;
		if ( ( m_pBuffer[ i ] & ( 1U << 25U ) ) != 0 )
			return MIN_NUM_BITS * i + 25;
		if ( ( m_pBuffer[ i ] & ( 1U << 26U ) ) != 0 )
			return MIN_NUM_BITS * i + 26;
		if ( ( m_pBuffer[ i ] & ( 1U << 27U ) ) != 0 )
			return MIN_NUM_BITS * i + 27;
		if ( ( m_pBuffer[ i ] & ( 1U << 28U ) ) != 0 )
			return MIN_NUM_BITS * i + 28;
		if ( ( m_pBuffer[ i ] & ( 1U << 29U ) ) != 0 )
			return MIN_NUM_BITS * i + 29;
		if ( ( m_pBuffer[ i ] & ( 1U << 30U ) ) != 0 )
			return MIN_NUM_BITS * i + 30;
		if ( ( m_pBuffer[ i ] & ( 1U << 31U ) ) != 0 )
			return MIN_NUM_BITS * i + 31;
	}

	return -1;
}

TINT TBitArray::GetBit( TINT a_iIndex ) const
{
	TASSERT( a_iIndex < m_iSize );

	return ( m_pBuffer[ a_iIndex / MIN_NUM_BITS ] & ( 1 << ( a_iIndex % m_iSize ) ) );
}

void TBitArray::ClearBit( TINT a_iIndex )
{
	TASSERT( a_iIndex < m_iSize );

	TUINT uiBit = 1 << ( a_iIndex % m_iSize );
	m_pBuffer[ a_iIndex / MIN_NUM_BITS ] &= ~uiBit;
}

void TBitArray::ToggleBit( TINT a_iIndex )
{
	TASSERT( a_iIndex < m_iSize );

	TUINT uiBit = 1 << ( a_iIndex % m_iSize );
	m_pBuffer[ a_iIndex / MIN_NUM_BITS ] ^= uiBit;
}

void TBitArray::SetBit( TINT a_iIndex )
{
	TASSERT( a_iIndex < m_iSize );

	TUINT uiBit = 1 << ( a_iIndex % m_iSize );
	m_pBuffer[ a_iIndex / MIN_NUM_BITS ] |= uiBit;
}

void TBitArray::SetBit( TINT a_iIndex, TINT a_iValue )
{
	TASSERT( a_iIndex < m_iSize );

	TUINT uiBit = 1 << ( a_iIndex % m_iSize );

	if ( a_iValue == 0 ) m_pBuffer[ a_iIndex / MIN_NUM_BITS ] &= ~uiBit;
	else
		m_pBuffer[ a_iIndex / MIN_NUM_BITS ] |= uiBit;
}

void TBitArray::Reset()
{
	TUINT uiNumElements = GetNumAllocatedBytes() / sizeof( TUINT32 );

	for ( TUINT i = 0; i < uiNumElements; i++ )
	{
		m_pBuffer[ i ] = m_uiInitialisedWith;
	}
}

void TBitArray::Destroy()
{
	if ( m_pBuffer )
		delete m_pBuffer;

	m_pBuffer = TNULL;
}

void TBitArray::Create( TINT a_iNumBits, TINT a_iInitialiseWith )
{
	m_iSize             = a_iNumBits;
	m_uiInitialisedWith = ( a_iInitialiseWith == 0 ) ? 0 : 0xFFFFFFFF;

	m_pBuffer = new TUINT32[ GetNumAllocatedBytes() / sizeof( TUINT32 ) ];
	TVALIDPTR( m_pBuffer );

	Reset();
}

TOSHI_NAMESPACE_END
