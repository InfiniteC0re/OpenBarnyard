#include "ToshiPCH.h"
#include "T2Atomic_WASM.h"

#include <emscripten/threading.h>
#include <limits.h>
#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TINT32 T2Atomic32::Load( MemoryOrders a_eOrder ) const
{
	return __atomic_load_n( &m_iValue, a_eOrder == MEMORY_ORDER_STRONG ? __ATOMIC_SEQ_CST : __ATOMIC_RELAXED );
}

TINT32 T2Atomic32::Exchange( TINT32 a_iValue )
{
	return __atomic_exchange_n( &m_iValue, a_iValue, __ATOMIC_SEQ_CST );
}

TINT32 T2Atomic32::Add( TINT32 a_iValue )
{
	return __atomic_fetch_add( &m_iValue, a_iValue, __ATOMIC_SEQ_CST );
}

TINT32 T2Atomic32::Or( TINT32 a_iMask )
{
	return __atomic_fetch_or( &m_iValue, a_iMask, __ATOMIC_SEQ_CST );
}

TINT32 T2Atomic32::And( TINT32 a_iMask )
{
	return __atomic_fetch_and( &m_iValue, a_iMask, __ATOMIC_SEQ_CST );
}

TINT32 T2Atomic32::CompareExchange( TINT32 a_iValue, TINT32 a_iCompare )
{
	TINT32 iExpected = a_iCompare;
	__atomic_compare_exchange_n( &m_iValue, &iExpected, a_iValue, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
	return iExpected;
}

void T2Atomic32::WaitForChange( TINT32 a_iValue )
{
	while ( Load( T2Atomic32::MEMORY_ORDER_WEAK ) == a_iValue )
	{
		emscripten_futex_wait( Address(), a_iValue, HUGE_VAL );
	}
}

void T2Atomic32::WaitForChange()
{
	WaitForChange( Load( T2Atomic32::MEMORY_ORDER_WEAK ) );
}

void T2Atomic32::Signal()
{
	emscripten_futex_wake( Address(), 1 );
}

TOSHI_NAMESPACE_END
