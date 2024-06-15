#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi {

	template <TUINT Size>
	class T2FixedString
	{
	public:
		TSTATICASSERT( Size > 0 );
		TSTATICASSERT( Size <= 4096 );

	public:
		constexpr T2FixedString()
		{
			m_szBuffer[ 0 ] = '\0';
		}

		T2FixedString( const TCHAR* a_szString )
		{
			Copy( a_szString );
		}

		void Copy( const TCHAR* a_szString )
		{
			T2String8::CopySafe( m_szBuffer, a_szString, Size );
		}

		/**
		 * Parses a line from specified buffer and saves it.
		 * @param a_szString buffer to parse buffer
		 * @param a_uiSize size of the a_szString buffer
		 * @param a_pStringLength if not TNULL, string length is saved there
		 * @param a_bTrimSpaces if TTRUE, removes spaces at the start and at the end
		 * @return number of characters to skip in the buffer
		 */
		TINT ParseLine( const TWCHAR* a_szString, TINT a_uiSize = -1, TINT* a_pStringLength = TNULL, TBOOL a_bTrimStartSpaces = TTRUE, TBOOL a_bTrimEndSpaces = TTRUE )
		{
			TINT uiPos = 0;
			TINT uiStringPos = 0;
			TBOOL bTextStarted = TFALSE;

			if ( a_uiSize == -1 )
				a_uiSize = INT_MAX;

			while ( uiPos < Size && uiStringPos < a_uiSize && a_szString[ uiStringPos ] != '\0' )
			{
				if ( a_szString[ uiStringPos ] != '\n' )
				{
					// Trim spaced at the start of the string
					if ( !bTextStarted && a_bTrimStartSpaces )
					{
						bTextStarted = !std::isspace( a_szString[ uiStringPos ] );

						if ( !bTextStarted )
						{
							uiStringPos++;
							continue;
						}
					}

					m_szBuffer[ uiPos++ ] = a_szString[ uiStringPos++ ];
				}
				else
				{
					break;
				}
			}

			uiPos -= 1;

			if ( a_bTrimEndSpaces )
			{
				while ( uiPos >= 0 && m_szBuffer[ uiPos ] != '\n' && std::isspace( m_szBuffer[ uiPos ] ) )
				{
					uiPos--;
				}
			}

			m_szBuffer[ uiPos + 1 ] = '\0';

			if ( a_pStringLength )
			{
				*a_pStringLength = uiPos + 1;
			}

			return uiStringPos + 1;
		}

		void Format( const TCHAR* a_szFormat, ... )
		{
			va_list args;

			va_start( args, a_szFormat );
			T2String8::FormatV( m_szBuffer, Size, a_szFormat, args );
			va_end( args );

			m_szBuffer[ Size - 1 ] = '\0';
		}

		void FormatV( const TCHAR* a_szFormat, va_list a_Args )
		{
			T2String8::FormatV( m_szBuffer, Size, a_szFormat, a_Args );
			m_szBuffer[ Size - 1 ] = '\0';
		}

		void Append( const TCHAR* a_szString )
		{
			T2String8::Concat( m_szBuffer, a_szString );
		}

		void Clear()
		{
			m_szBuffer[ 0 ] = '\0';
		}

		TUINT Length()
		{
			return T2String8::Length( m_szBuffer );
		}

		constexpr TCHAR* Get()
		{
			return m_szBuffer;
		}

		constexpr const TCHAR* Get() const
		{
			return m_szBuffer;
		}

		constexpr TCHAR* Get( TUINT a_uiIndex )
		{
			TASSERT( a_uiIndex < Size );
			return &m_szBuffer[ a_uiIndex ];
		}

		constexpr const TCHAR* Get( TUINT a_uiIndex ) const
		{
			TASSERT( a_uiIndex < Size );
			return &m_szBuffer[ a_uiIndex ];
		}

		constexpr TCHAR& operator[]( TUINT a_uiIndex )
		{
			return *Get( a_uiIndex );
		}

		constexpr const TCHAR& operator[]( TUINT a_uiIndex ) const
		{
			return *Get( a_uiIndex );
		}

	private:
		TCHAR m_szBuffer[ Size ];
	};

	using T2FixedString32 = T2FixedString<32>;
	using T2FixedString64 = T2FixedString<64>;
	using T2FixedString128 = T2FixedString<128>;
	using T2FixedString256 = T2FixedString<256>;
	using T2FixedString512 = T2FixedString<512>;
	using T2FixedString1024 = T2FixedString<1024>;
	using T2FixedString2048 = T2FixedString<2048>;

}