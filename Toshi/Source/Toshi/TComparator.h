#pragma once

TOSHI_NAMESPACE_START

template <class T>
class TComparator
{
public:
	static TBOOL IsEqual( const T& a, const T& b )
	{
		return a == b;
	}

	static TBOOL IsGreater( const T& a, const T& b )
	{
		return a > b;
	}

	static TBOOL IsLess( const T& a, const T& b )
	{
		return a < b;
	}

	static TBOOL IsLessOrEqual( const T& a, const T& b )
	{
		return a <= b;
	}

	static TBOOL IsGreaterOrEqual( const T& a, const T& b )
	{
		return a >= b;
	}
};

TOSHI_NAMESPACE_END
