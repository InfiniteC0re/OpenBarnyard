#include "ToshiPCH.h"
#include "T2String8Parser.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	T2String8Parser::T2String8Parser( T2ConstString8 a_pchCommandLine, TSIZE a_uiBufferSize /*= -1 */ )
	{
		SetBuffer( a_pchCommandLine, a_uiBufferSize );
	}

	T2String8Parser::T2String8Parser()
	{
		m_uiBufferSize = 0;
	}

	void T2String8Parser::SetBuffer( T2ConstString8 a_pchCommandLine, TSIZE a_uiBufferSize /*= -1 */ )
	{
		m_Buffer = a_pchCommandLine;
		m_BufferPos = a_pchCommandLine;

		if ( a_pchCommandLine )
			m_uiBufferSize = ( a_uiBufferSize == -1 ) ? a_pchCommandLine.Length() : a_uiBufferSize;
		else
			m_uiBufferSize = 0;

		m_BufferEnd = m_Buffer + m_uiBufferSize;
	}

	void T2String8Parser::Reset()
	{
		m_BufferPos = m_Buffer;
	}

	TSIZE T2String8Parser::Tell() const
	{
		return m_BufferPos - m_Buffer;
	}

	void T2String8Parser::Set( TSIZE a_uiPos )
	{
		m_BufferPos = m_Buffer + a_uiPos;
	}

	TBOOL T2String8Parser::PeekToken( TCHAR* a_pszToken, TSIZE a_uiTokenMaxSize )
	{
		TASSERT( TNULL != a_pszToken );
		TASSERT( a_uiTokenMaxSize > 1 );

		TSIZE uiSizeLeft = a_uiTokenMaxSize;
		TCHAR* pchCopyCursor = a_pszToken;
		T2ConstString8 cursor = m_BufferPos.SkipSpaces();

		TBOOL bIsInQuotes = TFALSE;
		TBOOL bIsPrevCharEscape = TFALSE;
		TBOOL bIsValid = TFALSE;

		// Copy until we have the last byte for the \0 character or the buffer is over
		while ( uiSizeLeft > 1 && cursor < m_BufferEnd )
		{
			TCHAR cChar = *cursor;
			TBOOL bIsEscapeChar = cChar == '\\';
			TBOOL bIsBreakToken = isspace( cChar );

			if ( !bIsPrevCharEscape && ( cChar == '"' || cChar == '\'' ) )
			{
				bIsInQuotes = !bIsInQuotes;

				if ( !bIsInQuotes )
				{
					// The quotes are closed so the token is over
					cursor += 1;
					bIsValid = TTRUE;
					break;
				}
			}
			else if ( !bIsInQuotes && bIsBreakToken )
			{
				// Break (stop) character is found and it's not in quotes
				cursor += 1;
				bIsValid = TTRUE;
				break;
			}
			else if ( !bIsEscapeChar )
			{
				// Copy the character
				*( pchCopyCursor++ ) = cChar;
			}

			cursor += 1;
			bIsPrevCharEscape = bIsEscapeChar;
		}

		if ( cursor >= m_BufferEnd )
			bIsValid = TTRUE;

		m_BufferLastPeek = cursor;
		*pchCopyCursor = '\0';
		return bIsValid;
	}

	TBOOL T2String8Parser::ReadToken( TCHAR* a_pszToken, TSIZE a_uiTokenMaxSize )
	{
		TBOOL bResult = PeekToken( a_pszToken, a_uiTokenMaxSize );
		m_BufferPos = m_BufferLastPeek;

		return bResult;
	}

	TBOOL T2String8Parser::IsOver() const
	{
		return m_BufferPos >= m_BufferEnd;
	}

}
