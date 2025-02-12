#pragma once

TOSHI_NAMESPACE_START

template <typename T, TINT C>
class TStack
{
public:
	TStack()  = default;
	~TStack() = default;

	void Push( const T& a_item = T() )
	{
		m_iTop++;
		if ( IsFull() ) return;
		AtUnsafe( m_iTop ) = a_item;
	}

	T& PushNull()
	{
		m_iTop++;
		return Top();
	}

	T& Pop()
	{
		T& item = AtUnsafe( m_iTop );
		m_iTop--;
		if ( IsEmpty() ) m_iTop = 0;
		return item;
	}

	TBOOL IsFull() const
	{
		return m_iTop == ( C - 1 );
	}

	T& Top()
	{
		return AtUnsafe( m_iTop );
	}

	TBOOL IsEmpty() const
	{
		return m_iTop == -1;
	}

	void Reset()
	{
		m_iTop = -1;
	}

	TINT Count() const
	{
		return m_iTop + 1;
	}

	T& Begin()
	{
		return AtUnsafe( 0 );
	}

	const T& Begin() const
	{
		return AtUnsafe( 0 );
	}

	T& End()
	{
		return AtUnsafe( m_iTop );
	}

	const T& End() const
	{
		return AtUnsafe( m_iTop );
	}

private:
	TFORCEINLINE constexpr T& AtUnsafe( TUINT a_uiIndex )
	{
		return *( (T*)( m_aBuffer ) + a_uiIndex );
	}

	TFORCEINLINE constexpr const T& AtUnsafe( TUINT a_uiIndex ) const
	{
		return *( (const T*)( m_aBuffer ) + a_uiIndex );
	}

private:
	TINT  m_iTop = -1;
	TBYTE m_aBuffer[ sizeof( T ) * C ];
};

TOSHI_NAMESPACE_END
