#pragma once
#include "Toshi/Defines.h"
#include "Toshi/T2Iterator.h"

TOSHI_NAMESPACE_START

template <TUINT NUM_BITS>
class TBitField
{
public:
	using ValueType                        = TUINT32;
	static constexpr TUINT MIN_NUM_BITS    = 8 * sizeof( ValueType );
	static constexpr TUINT BIT_SUBNUM_MASK = MIN_NUM_BITS - 1;

public:
	TBitField()
	{
		T2_FOREACH_ARRAY( m_Values, i )
		{
			m_Values[ i ] = 0;
		}
	}

	// TBitField<32> 00413220
	// $Barnyard: FUNCTION 00413220
	void Set( TINT a_iBitNum, TBOOL a_bSet )
	{
		TASSERT( a_iBitNum >= 0 );
		TASSERT( a_iBitNum < NUM_BITS );

		TUINT uiBit = 1 << ( a_iBitNum % NUM_BITS );

		if ( a_bSet ) m_Values[ a_iBitNum / MIN_NUM_BITS ] |= uiBit;
		else
			m_Values[ a_iBitNum / MIN_NUM_BITS ] &= ~uiBit;
	}

	// TBitField<32> 0040e940
	// $Barnyard: FUNCTION 0040e940
	TBOOL IsSet( TINT a_iBitNum ) const
	{
		TASSERT( a_iBitNum >= 0 );
		TASSERT( a_iBitNum < NUM_BITS );

		return ( m_Values[ a_iBitNum / MIN_NUM_BITS ] & ( 1 << ( a_iBitNum % NUM_BITS ) ) ) != 0;
	}

	// NOTE: TBitField<32> instance used on AUnitFlagSet masks by AInteraction::UpdatePriority
	// $Barnyard: FUNCTION 0059e630
	TBOOL IsEmpty() const
	{
		T2_FOREACH_ARRAY( m_Values, i )
		{
			if ( m_Values[ i ] != 0 )
				return TFALSE;
		}

		return TTRUE;
	}

private:
	ValueType m_Values[ ( NUM_BITS + BIT_SUBNUM_MASK ) / MIN_NUM_BITS ];
};

TOSHI_NAMESPACE_END
