#pragma once
#include <type_traits>
#include <cstdint>

template <size_t n, typename = void>
struct numeric_type_from_size_impl;

template <size_t n>
struct numeric_type_from_size_impl<n, typename std::enable_if<(n == 1)>::type> {
	using type = uint8_t;
};

template <size_t n>
struct numeric_type_from_size_impl<n, typename std::enable_if<(n == 2)>::type> {
	using type = uint16_t;
};

template <size_t n>
struct numeric_type_from_size_impl<n, typename std::enable_if<(n == 4)>::type> {
	using type = uint32_t;
};

template <size_t n>
struct numeric_type_from_size_impl<n, typename std::enable_if<(n == 8)>::type> {
	using type = uint64_t;
};

#define DEFINE_T2FLAGS(ENUM) \
__forceinline Toshi::T2Flags<ENUM>::NumericType operator|(ENUM first, ENUM second) \
{ \
	return TSTATICCAST(Toshi::T2Flags<ENUM>::NumericType, first) | TSTATICCAST(Toshi::T2Flags<ENUM>::NumericType, second); \
}

namespace Toshi {

	template <class T>
	class T2Flags
	{
	public:
		using NumericType = typename numeric_type_from_size_impl<sizeof(T)>::type;

	public:
		__forceinline T2Flags()
		{
			m_Value = 0;
		}

		__forceinline T2Flags(T flags)
		{
			m_Value = static_cast<NumericType>(flags);
		}

		__forceinline T2Flags(NumericType flags)
		{
			m_Value = flags;
		}

		__forceinline NumericType GetFlags() const
		{
			return m_Value;
		}

		__forceinline TBOOL IsZero() const
		{
			return m_Value == 0;
		}

		__forceinline void Set(T2Flags<T> flags)
		{
			m_Value |= flags.m_Value;
		}

		__forceinline void Unset(T2Flags<T> flags)
		{
			m_Value &= ~flags.m_Value;
		}

		__forceinline void Mask(T2Flags<T> flags)
		{
			m_Value &= flags.m_Value;
		}

		__forceinline void Toggle(T2Flags<T> flags)
		{
			if (!IsSet(flags))
				Set(flags);
			else
				UnSet(flags);
		}

		__forceinline TBOOL IsSet(T2Flags<T> flags) const
		{
			return (m_Value & flags.m_Value) == flags.m_Value;
		}

		__forceinline operator TBOOL() const
		{
			return m_Value != 0;
		}

		__forceinline T2Flags<T> operator|(T2Flags<T> flags) const
		{
			return T2Flags<T>(m_Value | flags.m_Value);
		}

		__forceinline T2Flags<T> operator&(T2Flags<T> flags) const
		{
			return T2Flags<T>(m_Value & flags.m_Value);
		}

		__forceinline void operator|=(T2Flags<T> flags)
		{
			m_Value |= flags.m_Value;
		}

		__forceinline void operator&=(T2Flags<T> flags)
		{
			m_Value &= flags.m_Value;
		}

		__forceinline T2Flags<T> operator|(NumericType flags) const
		{
			return T2Flags<T>(m_Value | flags);
		}

		__forceinline T2Flags<T> operator&(NumericType flags) const
		{
			return T2Flags<T>(m_Value & flags);
		}

		__forceinline void operator|=(NumericType flags)
		{
			m_Value |= flags;
		}

		__forceinline void operator&=(NumericType flags)
		{
			m_Value &= flags;
		}

	private:
		NumericType m_Value;
	};

}
