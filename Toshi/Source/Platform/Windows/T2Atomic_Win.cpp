#include "ToshiPCH.h"
#include "T2Atomic_Win.h"

#include <Windows.h>

#pragma comment( lib, "synchronization.lib" )

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TINT32 T2Atomic32::Load( MemoryOrders a_eOrder ) const
{
	TINT32 iValue = __iso_volatile_load32( &m_iValue );

	if ( a_eOrder == MEMORY_ORDER_STRONG )
		_ReadWriteBarrier();

	return iValue;
}

TINT32 T2Atomic32::Add( TINT32 a_iValue )
{
	return _InterlockedExchangeAdd( TREINTERPRETCAST( volatile long*, &m_iValue ), a_iValue );
}

TINT32 T2Atomic32::Or( TINT32 a_iMask )
{
	return _InterlockedOr( TREINTERPRETCAST( volatile long*, &m_iValue ), a_iMask );
}

TINT32 T2Atomic32::And( TINT32 a_iMask )
{
	return _InterlockedAnd( TREINTERPRETCAST( volatile long*, &m_iValue ), a_iMask );
}

TINT32 T2Atomic32::Exchange( TINT32 a_iValue )
{
	return _InterlockedExchange( TREINTERPRETCAST( volatile long*, &m_iValue ), a_iValue );
}

TINT32 T2Atomic32::CompareExchange( TINT32 a_iValue, TINT32 a_iCompare )
{
	return _InterlockedCompareExchange( TREINTERPRETCAST( volatile long*, &m_iValue ), a_iValue, a_iCompare );
}

void T2Atomic32::WaitForChange( TINT32 a_iValue )
{
	while ( Load( T2Atomic32::MEMORY_ORDER_WEAK ) == a_iValue )
	{
		WaitOnAddress( Address(), &a_iValue, sizeof( TINT32 ), INFINITE );
	}
}

void T2Atomic32::WaitForChange()
{
	TINT32 iValue = Load( T2Atomic32::MEMORY_ORDER_WEAK );

	do
	{
		WaitOnAddress( Address(), &iValue, sizeof( TINT32 ), INFINITE );

	} while ( Load( T2Atomic32::MEMORY_ORDER_WEAK ) == iValue );
}

void T2Atomic32::Signal()
{
	WakeByAddressSingle( (void*)Address() );
}

TOSHI_NAMESPACE_END
