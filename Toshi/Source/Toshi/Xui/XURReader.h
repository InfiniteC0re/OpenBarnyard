#pragma once
#include "Toshi/Xui/TXUIElement.h"
#include "Toshi/Core/Endianness.h"

namespace Toshi
{
	class XURReader
	{
	public:
		XURReader(uint8_t*& buffer) : m_Buffer(buffer), m_ReadFlags(0) { }

		__forceinline void SetPos(uint8_t* buffer)
		{
			m_Buffer = buffer;
		}

		__forceinline void SeekFromCur(int offset)
		{
			m_Buffer += offset;
		}

		__forceinline uint8_t* GetPos()
		{
			return m_Buffer;
		}

		__forceinline void SetReadFlags(uint32_t readFlags)
		{
			m_ReadFlags = readFlags;
		}

		__forceinline uint32_t GetReadFlags()
		{
			return m_ReadFlags;
		}

		template <uint32_t numOfProps>
		__forceinline uint32_t ReadPropsInfo()
		{
			if constexpr (numOfProps > 8)
			{
				m_ReadFlags = ReadUInt16();
			}
			else
			{
				m_ReadFlags = ReadUInt8();
			}

			return m_ReadFlags;
		}

		__forceinline uint32_t ReadUInt32LE()
		{
			uint32_t result = PARSEDWORD(m_Buffer);
			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline uint16_t ReadUInt16LE()
		{
			uint16_t result = PARSEWORD(m_Buffer);
			m_Buffer += sizeof(uint16_t);
			return result;
		}

		__forceinline uint32_t ReadUInt32()
		{
			uint32_t result = PARSEDWORD_BIG(m_Buffer);
			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline uint16_t ReadUInt16()
		{
			uint16_t result = PARSEWORD_BIG(m_Buffer);
			m_Buffer += sizeof(uint16_t);
			return result;
		}

		__forceinline uint8_t ReadUInt8()
		{
			uint8_t result = *m_Buffer;
			m_Buffer += 1;
			return result;
		}

		__forceinline float ReadFloat()
		{
			float result = PARSEFLOAT_BIG(m_Buffer);
			m_Buffer += sizeof(float);
			return result;
		}

		__forceinline uint8_t ReadUInt8From32()
		{
			TASSERT(PARSEDWORD_BIG(m_Buffer) < (1 << 8));

			uint8_t result = *(m_Buffer + 3);
			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline uint16_t ReadUInt16From32()
		{
			TASSERT(PARSEDWORD_BIG(m_Buffer) < (1 << 16));
			uint16_t result = PARSEWORD_BIG(m_Buffer + 2);

			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline int32_t ReadInt32()
		{
			return ReadUInt32();
		}

		__forceinline int16_t ReadInt16()
		{
			return ReadUInt16();
		}

		__forceinline uint32_t ReadPackedUInt32()
		{
			uint32_t result;
			uint32_t firstByte = ReadUInt8();

			if (firstByte != 0xFF)
			{
				if (firstByte < 0xF0)
				{
					result = firstByte;
				}
				else
				{
					uint32_t secondByte = ReadUInt8();
					uint32_t highPart = (firstByte << 8) & 0x0F00;
					result = highPart | secondByte;
				}
			}
			else
			{
				result = ReadUInt32();
			}

			return result;
		}

		__forceinline XUIEPTBool ReadEPTBool()
		{
			return ReadUInt8();
		}

		__forceinline XUIEPTUInt8 ReadEPTUInt8()
		{
			return ReadUInt8();
		}

		__forceinline XUIEPTInteger ReadEPTInteger()
		{
			return ReadInt32();
		}

		__forceinline XUIEPTUnsigned ReadEPTUnsigned()
		{
			return ReadUInt32();
		}

		__forceinline XUIEPTFloat ReadEPTFloat()
		{
			return ReadFloat();
		}

		__forceinline XUIEPTString ReadEPTString()
		{
			return ReadUInt16();
		}

		__forceinline XUIEPTShort32 ReadEPTShort32()
		{
			return ReadUInt16From32();
		}

		__forceinline XUIEPTUShort32 ReadEPTUShort32()
		{
			return ReadUInt16From32();
		}

		__forceinline XUIEPTUColor ReadEPTColor()
		{
			return ReadUInt32();
		}

		__forceinline XUIEPTVector ReadEPTVector()
		{
			return ReadEPTUShort32();
		}

		__forceinline XUIEPTQuaternion ReadEPTQuaternion()
		{
			return ReadEPTUShort32();
		}

		__forceinline bool ShouldReadThisProp(int propId)
		{
			return (m_ReadFlags & (1 << propId));
		}

		template<int EPT, typename T>
		__forceinline bool ReadProperty(int propId, T& outValue)
		{
			if (ShouldReadThisProp(propId))
			{
				if constexpr (EPT == XUI_EPT_BOOL)
				{
					outValue = ReadEPTBool();
				}
				else if constexpr (EPT == XUI_EPT_INTEGER)
				{
					outValue = ReadEPTInteger();
				}
				else if constexpr (EPT == XUI_EPT_UNSIGNED)
				{
					outValue = ReadEPTUnsigned();
				}
				else if constexpr (EPT == XUI_EPT_FLOAT)
				{
					outValue = ReadEPTFloat();
				}
				else if constexpr (EPT == XUI_EPT_STRING)
				{
					outValue = ReadEPTString();
				}
				else if constexpr (EPT == XUI_EPT_COLOR)
				{
					outValue = ReadEPTColor();
				}
				else if constexpr (EPT == XUI_EPT_VECTOR)
				{
					outValue = ReadEPTVector();
				}
				else if constexpr (EPT == XUI_EPT_QUATERNION)
				{
					outValue = ReadEPTQuaternion();
				}
				else if constexpr (EPT == XUI_EPT_OBJECT)
				{
					TASSERT(false && "Not implemented");
				}
				else if constexpr (EPT == XUI_EPT_CUSTOM)
				{
					outValue = ReadEPTUnsigned();
				}
				else if constexpr (EPT == XUI_EPT_SHORT32)
				{
					outValue = ReadEPTShort32();
				}
				else if constexpr (EPT == XUI_EPT_USHORT32)
				{
					outValue = ReadEPTUShort32();
				}

				return true;
			}

			return false;
		}

	private:
		uint8_t*& m_Buffer;
		uint32_t m_ReadFlags;
	};
}