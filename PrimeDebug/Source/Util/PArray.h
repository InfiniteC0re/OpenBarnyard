#pragma once
#include "PrimeDebug.h"

#include <string.h>

// A growable array of plain values (pointers, small structs copied with memcpy), as Prime's crArray
template <class T>
class PArray
{
public:
	PArray()
	    : m_pData( TNULL )
	    , m_iSize( 0 )
	    , m_iCapacity( 0 )
	{}

	~PArray() { Free(); }

	T&       operator[]( TINT a_iIndex ) { return m_pData[ a_iIndex ]; }
	const T& operator[]( TINT a_iIndex ) const { return m_pData[ a_iIndex ]; }

	TINT Size() const { return m_iSize; }
	T*   Data() { return m_pData; }

	void Free()
	{
		delete[] m_pData;
		m_pData     = TNULL;
		m_iSize     = 0;
		m_iCapacity = 0;
	}

	void Reserve( TINT a_iCapacity )
	{
		if ( a_iCapacity <= m_iCapacity )
			return;

		T* pData = new T[ a_iCapacity ];

		if ( m_pData )
		{
			memcpy( pData, m_pData, m_iSize * sizeof( T ) );
			delete[] m_pData;
		}

		m_pData     = pData;
		m_iCapacity = a_iCapacity;
	}

	void SetSize( TINT a_iSize )
	{
		if ( a_iSize > m_iCapacity )
			Reserve( a_iSize );

		m_iSize = a_iSize;
	}

	void PushBack( const T& a_rValue )
	{
		if ( m_iSize == m_iCapacity )
			Reserve( m_iCapacity ? m_iCapacity * 2 : 8 );

		m_pData[ m_iSize++ ] = a_rValue;
	}

	// Makes room at a_iIndex (the slot is left as it was)
	void InsertAt( TINT a_iIndex )
	{
		if ( m_iSize == m_iCapacity )
			Reserve( m_iCapacity ? m_iCapacity * 2 : 8 );

		memmove( m_pData + a_iIndex + 1, m_pData + a_iIndex, ( m_iSize - a_iIndex ) * sizeof( T ) );
		m_iSize++;
	}

private:
	T*   m_pData;
	TINT m_iSize;
	TINT m_iCapacity;
};
