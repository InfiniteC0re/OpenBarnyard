#pragma once

TOSHI_NAMESPACE_START

template <typename T, TINT C>
class TStack
{
public:
	TStack() = default;
	~TStack() = default;

	void Push( const T& a_item = T() )
	{
		m_iTop++;
		if ( IsFull() ) return;
		AtUnsafe( m_iTop ) = a_item;
	}

	void PushNull()
	{
		m_iTop++;
	}

	T& Pop()
	{
		T& item = AtUnsafe( m_iTop );
		m_iTop--;
		if ( IsEmpty() ) m_iTop = 0;
		return item;
	}

	TBOOL IsFull()
	{
		return m_iTop == ( C - 1 );
	}

	T& Top()
	{
		return AtUnsafe( m_iTop );
	}

	TBOOL IsEmpty()
	{
		return m_iTop == -1;
	}

	void Reset()
	{
		m_iTop = 0;
	}

private:
	TFORCEINLINE constexpr T& AtUnsafe( TUINT a_uiIndex )
	{
		return *( (T*)(m_aBuffer) + a_uiIndex );
	}

	TFORCEINLINE constexpr const T& AtUnsafe( TUINT a_uiIndex ) const
	{
		return *( (const T*)(m_aBuffer) + a_uiIndex );
	}

private:
	TINT m_iTop = -1;
	TBYTE m_aBuffer[ sizeof( T ) * C ];
};

TOSHI_NAMESPACE_END
