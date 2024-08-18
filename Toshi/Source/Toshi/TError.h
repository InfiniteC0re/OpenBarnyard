#pragma once

TOSHI_NAMESPACE_START

class TError : public TSingleton<TError>
{
public:
	TError( TUINT32 size1, TUINT32 size2 );
	~TError();

	void         AddError( const TCHAR* error, ... );
	const TCHAR* GetError( TUINT32 index ) { return index > -1 && index < m_currentSize2 ? &m_Buffer1[ m_Buffer2[ index ] ] : "invalid error id"; }

	TCHAR*  m_Buffer1;      // 0x0
	TUINT32 m_size1;        // 0x4
	TINT*   m_Buffer2;      // 0x8
	TUINT32 m_size2;        // 0xC
	TUINT32 m_currentSize;  // 0x10
	TUINT32 m_currentSize2; // 0x14
};

TOSHI_NAMESPACE_END
