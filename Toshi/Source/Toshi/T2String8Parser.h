#pragma once
#include "Toshi/T2String8.h"

namespace Toshi {

	class T2String8Parser
	{
	public:
		T2String8Parser();
		T2String8Parser( T2ConstString8 a_pchCommandLine, TSIZE a_uiBufferSize = -1 );

		void SetBuffer( T2ConstString8 a_pchCommandLine, TSIZE a_uiBufferSize = -1 );
		void Reset();

		TSIZE Tell() const;
		void Set( TSIZE a_uiPos );

		TBOOL PeekToken( TCHAR* a_pszToken, TSIZE a_uiTokenMaxSize );
		TBOOL ReadToken( TCHAR* a_pszToken, TSIZE a_uiTokenMaxSize );

		TBOOL IsOver() const;

	private:
		T2ConstString8 m_Buffer;
		T2ConstString8 m_BufferPos;
		T2ConstString8 m_BufferEnd;
		T2ConstString8 m_BufferLastPeek;
		TSIZE m_uiBufferSize;
	};

}