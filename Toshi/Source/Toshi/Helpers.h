#pragma once

template <typename T>
void TSwapValues(T& a, T& b)
{
	T temp = std::move(a);
	a = std::move(b);
	b = std::move(temp);
}

constexpr TUINT32 TMakeFour(const TCHAR str[4])
{
	return ((str[3] << 24) | (str[2] << 16) | (str[1] << 8) | str[0]);
}