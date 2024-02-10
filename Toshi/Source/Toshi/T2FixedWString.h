#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/T2String16.h"

namespace Toshi {

	template <TUINT Size>
	class T2FixedWString
	{
	public:
		TSTATICASSERT(Size > 0);
		TSTATICASSERT(Size <= 4096);

	public:
		constexpr T2FixedWString()
		{
			m_szBuffer[0] = '\0';
		}

		T2FixedWString(const TWCHAR* a_szString)
		{
			T2String16::CopySafe(m_szBuffer, a_szString, Size);
		}

		/**
		 * Parses a line from specified buffer and saves it.
		 * @param a_szString buffer to parse buffer
		 * @param a_uiSize size of the a_szString buffer
		 * @param a_pStringLength if not TNULL, string length is saved there
		 * @param a_bTrimSpaces if TTRUE, removes spaces at the start and at the end
		 * @return number of characters to skip in the buffer
		 */
		TINT ParseLine(const TWCHAR* a_szString, TINT a_uiSize = -1, TINT* a_pStringLength = TNULL, TBOOL a_bTrimStartSpaces = TTRUE, TBOOL a_bTrimEndSpaces = TTRUE)
		{
			TINT uiPos = 0;
			TINT uiStringPos = 0;
			TBOOL bTextStarted = TFALSE;

			if (a_uiSize == -1)
				a_uiSize = INT_MAX;

			while (uiPos < Size && uiStringPos < a_uiSize && a_szString[uiStringPos] != L'\0')
			{
				if (a_szString[uiStringPos] != L'\n')
				{
					// Trim spaced at the start of the string
					if (!bTextStarted && a_bTrimStartSpaces)
					{
						bTextStarted = !iswspace(a_szString[uiStringPos]);

						if (!bTextStarted)
						{
							uiStringPos++;
							continue;
						}
					}

					m_szBuffer[uiPos++] = a_szString[uiStringPos++];
				}
				else
				{
					break;
				}
			}

			uiPos -= 1;

			if (a_bTrimEndSpaces)
			{
				while (uiPos >= 0 && m_szBuffer[uiPos] != '\n' && iswspace(m_szBuffer[uiPos]))
				{
					uiPos--;
				}
			}

			m_szBuffer[uiPos + 1] = '\0';

			if (a_pStringLength)
			{
				*a_pStringLength = uiPos + 1;
			}

			return uiStringPos + 1;
		}

		void Format(const TWCHAR* a_szFormat, ...)
		{
			va_list args;

			va_start(args, a_szFormat);
			T2String16::FormatV(m_szBuffer, Size, a_szFormat, args);
			va_end(args);

			m_szBuffer[Size - 1] = '\0';
		}

		void FormatV(const TWCHAR* a_szFormat, va_list a_Args)
		{
			T2String16::FormatV(m_szBuffer, Size, a_szFormat, a_Args);
			m_szBuffer[Size - 1] = '\0';
		}

		void Append(const TWCHAR* a_szString)
		{
			T2String16::Concat(m_szBuffer, a_szString);
		}

		void Clear()
		{
			m_szBuffer[0] = '\0';
		}

		TUINT Length()
		{
			return T2String16::Length(m_szBuffer);
		}

		constexpr TWCHAR* Get()
		{
			return m_szBuffer;
		}

		constexpr const TWCHAR* Get() const
		{
			return m_szBuffer;
		}

		constexpr TWCHAR* Get(TUINT a_uiIndex)
		{
			TASSERT(a_uiIndex < Size);
			return &m_szBuffer[a_uiIndex];
		}

		constexpr const TWCHAR* Get(TUINT a_uiIndex) const
		{
			TASSERT(a_uiIndex < Size);
			return &m_szBuffer[a_uiIndex];
		}

		constexpr TWCHAR& operator[](TUINT a_uiIndex)
		{
			return *Get(a_uiIndex);
		}

		constexpr const TWCHAR& operator[](TUINT a_uiIndex) const
		{
			return *Get(a_uiIndex);
		}

	private:
		TWCHAR m_szBuffer[Size];
	};

	using T2FixedWString32 = T2FixedWString<32>;
	using T2FixedWString64 = T2FixedWString<64>;
	using T2FixedWString128 = T2FixedWString<128>;
	using T2FixedWString256 = T2FixedWString<256>;
	using T2FixedWString512 = T2FixedWString<512>;
	using T2FixedWString1024 = T2FixedWString<1024>;
	using T2FixedWString2048 = T2FixedWString<2048>;

}