#include "ToshiPCH.h"
#include "TError.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

Toshi::TError::TError( TUINT32 size1, TUINT32 size2 )
{
	m_size1        = size1;
	m_size2        = size2;
	m_Buffer1      = new TCHAR[ size1 ];
	m_Buffer2      = new TINT[ size2 ];
	m_currentSize  = 0;
	m_currentSize2 = 0;
}

Toshi::TError::~TError()
{
	delete[] m_Buffer1;
	delete[] m_Buffer2;
}

void Toshi::TError::AddError( const TCHAR* error, ... )
{
	if ( m_currentSize2 < m_size2 && m_currentSize < m_size1 )
	{
		m_Buffer2[ m_currentSize2 ] = m_currentSize;

		va_list args;

		va_start( args, error );

		TCHAR str[ 0x400 ];

		T2String8::Format( str, sizeof( str ), error, args );

		TSIZE size = strlen( str );

		if ( size + m_currentSize <= m_size1 && TUtil::MemCopy( m_Buffer1 + m_currentSize, str, size ) )
		{
			m_currentSize2++;
			m_currentSize += size + 1;
		}
	}
}
