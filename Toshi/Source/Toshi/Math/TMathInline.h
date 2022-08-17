#pragma once
#include <math.h>
#include <float.h>

namespace TMath
{
	constexpr char TINT8_MAX = INT8_MAX;
	constexpr char TINT8_MIN = INT8_MIN;
	constexpr short TINT16_MAX = INT16_MAX;
	constexpr short TINT16_MIN = INT16_MIN;
	constexpr int TINT32_MAX = INT32_MAX;
	constexpr int TINT32_MIN = INT32_MIN;

	constexpr unsigned short MAXWCHAR = WCHAR_MAX;

	constexpr float TFLOAT_MAX = FLT_MAX;
	constexpr float TFLOAT_MIN = FLT_MIN;
	constexpr float FLOATEPSILON = FLT_EPSILON;

	constexpr float PI = (float)3.141592653589793238462643383279502884L;
	constexpr float TWO_PI = PI * 2;
	constexpr float HALF_PI = (float)(3.141592653589793238462643383279502884L / 2);
	constexpr float ONEOVERTWO_PI = 1 / TWO_PI;

	static bool IsFinite(float fVal) { return _finite(fVal) != 0; }
	static float Abs(float fVal) { return fabs(fVal); }
	static float Sqrt(float fVal) { return sqrt(fVal); }
};