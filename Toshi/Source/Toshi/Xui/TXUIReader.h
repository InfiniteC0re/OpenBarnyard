#pragma once
#include "Toshi/Xui/TXUIElement.h"
#include "Toshi/Core/Endianness.h"

namespace Toshi
{
	class TXUIReader
	{
	public:
		static constexpr bool sm_bIsBigEndian = true;

	public:
		TXUIReader(uint8_t*& buffer) : m_Buffer(buffer), m_ReadFlags(0) { }

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

		__forceinline uint32_t ReadUInt32()
		{
			uint32_t result;

			if constexpr (sm_bIsBigEndian)
			{
				result = PARSEDWORD_BIG(m_Buffer);
			}
			else
			{
				result = PARSEDWORD(m_Buffer);
			}

			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline uint16_t ReadUInt16()
		{
			uint16_t result;

			if constexpr (sm_bIsBigEndian)
			{
				result = PARSEWORD_BIG(m_Buffer);
			}
			else
			{
				result = PARSEWORD(m_Buffer);
			}

			m_Buffer += sizeof(uint16_t);
			return result;
		}

		__forceinline float ReadFloat()
		{
			float result;

			if constexpr (sm_bIsBigEndian)
			{
				result = PARSEFLOAT_BIG(m_Buffer);
			}
			else
			{
				result = PARSEFLOAT(m_Buffer);
			}

			m_Buffer += sizeof(float);
			return result;
		}

		__forceinline uint8_t ReadUInt8()
		{
			bool result = *m_Buffer;
			m_Buffer += 1;
			return result;
		}

		__forceinline uint8_t ReadUInt8From32()
		{
			if constexpr (sm_bIsBigEndian)
			{
				TASSERT(PARSEDWORD_BIG(m_Buffer) < (1 << 8));
			}
			else
			{
				TASSERT(PARSEDWORD(m_Buffer) < (1 << 8));
			}

			bool result = *m_Buffer;
			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline uint16_t ReadUInt16From32()
		{
			uint16_t result;

			if constexpr (sm_bIsBigEndian)
			{
				TASSERT(PARSEDWORD_BIG(m_Buffer) < (1 << 16));
				result = PARSEWORD_BIG(m_Buffer + 2);
			}
			else
			{
				TASSERT(PARSEDWORD(m_Buffer) < (1 << 16));
				result = PARSEWORD(m_Buffer);
			}

			m_Buffer += sizeof(uint32_t);
			return result;
		}

		__forceinline bool ShouldReadThisProp(XURXUIObjectData::PropType propType)
		{
			return (m_ReadFlags & (1 << propType));
		}

		__forceinline bool ReadProp32(XURXUIObjectData::PropType propType, uint32_t& out)
		{
			if (ShouldReadThisProp(propType))
			{
				out = ReadUInt32();
				return true;
			}

			return false;
		}

		__forceinline bool ReadProp16(XURXUIObjectData::PropType propType, uint16_t& out)
		{
			if (ShouldReadThisProp(propType))
			{
				out = ReadUInt16();
				return true;
			}

			return false;
		}

		template<class T = uint32_t>
		__forceinline bool ReadProp16From32(XURXUIObjectData::PropType propType, T& out)
		{
			if (ShouldReadThisProp(propType))
			{
				out = ReadUInt16From32();
				return true;
			}

			return false;
		}

		__forceinline bool ReadProp8(XURXUIObjectData::PropType propType, uint8_t& out)
		{
			if (ShouldReadThisProp(propType))
			{
				out = ReadUInt8();
				return true;
			}

			return false;
		}

		template<class T = uint32_t>
		__forceinline bool ReadProp8From32(XURXUIObjectData::PropType propType, T& out)
		{
			if (ShouldReadThisProp(propType))
			{
				out = ReadUInt8From32();
				return true;
			}

			return false;
		}

		__forceinline bool ReadPropFloat(XURXUIObjectData::PropType propType, float& out)
		{
			if (ShouldReadThisProp(propType))
			{
				out = ReadFloat();
				return true;
			}

			return false;
		}

		__forceinline int32_t ReadInt32()
		{
			return ReadUInt32();
		}

		__forceinline int16_t ReadInt16()
		{
			return ReadUInt16();
		}

	private:
		uint8_t*& m_Buffer;
		uint32_t m_ReadFlags;
	};
}