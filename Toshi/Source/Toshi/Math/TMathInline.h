#pragma once

class TMath
{
public:

	static constexpr char TINT8_MAX = INT8_MAX;
	static constexpr char TINT8_MIN = INT8_MIN;
	static constexpr short TINT16_MAX = INT16_MAX;
	static constexpr short TINT16_MIN = INT16_MIN;
	static constexpr int TINT32_MAX = INT32_MAX;
	static constexpr int TINT32_MIN = INT32_MIN;

	static constexpr unsigned short MAXWCHAR = WCHAR_MAX;

	static constexpr float TFLOAT_MAX = FLT_MAX;
	static constexpr float TFLOAT_MIN = FLT_MIN;
	static constexpr float FLOATEPSILON = FLT_EPSILON;

	static constexpr float PI = 3.141592653589793238462643383279502884L;
	static constexpr float TWO_PI = PI * 2;
	static constexpr float HALF_PI = 3.141592653589793238462643383279502884L / 2;
	static constexpr float ONEOVERTWO_PI = 1 / TWO_PI;

	static bool IsFinite(float fVal) { return _finite(fVal) != 0; }
	static float Abs(float fVal) { return abs(fVal); }
	static float Sqrt(float fVal) { return sqrt(fVal); }

};