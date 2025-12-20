#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: provides an ability to utilize atomic numbers.
// Any operations but loading a value use MEMORY_ORDER_STRONG memory order.
//-----------------------------------------------------------------------------
class TALIGNAS(4) T2Atomic32
{
public:
	enum MemoryOrders
	{
		MEMORY_ORDER_STRONG,
		MEMORY_ORDER_WEAK
	};

public:
	constexpr T2Atomic32( TINT32 a_iValue = 0 )
	    : m_iValue( a_iValue ) {}

	~T2Atomic32() = default;

	TINT32 Exchange( TINT32 a_iValue );
	TINT32 CompareExchange( TINT32 a_iValue, TINT32 a_iCompare );

	TINT32 Add( TINT32 a_iValue );
	TINT32 Or( TINT32 a_iMask );
	TINT32 And( TINT32 a_iMask );

	// Returns value of the variable
	TINT32 Load( MemoryOrders a_eOrder = MEMORY_ORDER_STRONG ) const;

	void WaitForChange();
	void WaitForChange( TINT32 a_iValue );
	void Signal();

	volatile TINT32* Address() { return &m_iValue; }
	volatile const TINT32* Address() const { return &m_iValue; }

	TINT32 operator+( TINT32 a_iValue ) const { return Load() + a_iValue; }
	TINT32 operator-( TINT32 a_iValue ) const { return Load() - a_iValue; }
	TINT32 operator*( TINT32 a_iValue ) const { return Load() * a_iValue; }
	TINT32 operator/( TINT32 a_iValue ) const { return Load() / a_iValue; }
	TINT32 operator|( TINT32 a_iMask ) const { return Load() | a_iMask; }
	TINT32 operator&( TINT32 a_iMask ) const { return Load() & a_iMask; }

	T2Atomic32& operator+=( TINT32 a_iValue ) { Add( a_iValue ); return *this; }
	T2Atomic32& operator-=( TINT32 a_iValue ) { Add( -a_iValue ); return *this; }
	T2Atomic32& operator|=( TINT32 a_iMask ) { Or( a_iMask ); return *this; }
	T2Atomic32& operator&=( TINT32 a_iMask ) { And( a_iMask ); return *this; }

private:
	volatile TINT32 m_iValue;
};

TOSHI_NAMESPACE_END
