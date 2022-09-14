#pragma once

namespace Toshi
{
	using TMathFloating = float;
}

namespace TMath
{
	// can be used to align size_t value to 4
	constexpr size_t SIZE_T_ALIGN_VALUE = UINTPTR_MAX - (sizeof(uint32_t) - 1);
	
	// can be used to align uint32_t value to 4 
	constexpr size_t UINT32_ALIGN_VALUE = UINT32_MAX - (sizeof(uint32_t) - 1);

	constexpr char TINT8_MAX = INT8_MAX;
	constexpr char TINT8_MIN = INT8_MIN;
	constexpr unsigned char TUINT8_MAX = UINT8_MAX;

	constexpr short TINT16_MAX = INT16_MAX;
	constexpr short TINT16_MIN = INT16_MIN;
	constexpr short TUINT16_MAX = UINT16_MAX;

	constexpr int TINT32_MAX = INT32_MAX;
	constexpr int TINT32_MIN = INT32_MIN;
	constexpr int TUINT32_MAX = UINT32_MAX;

	constexpr unsigned short MAXWCHAR = WCHAR_MAX;

	constexpr float TFLOAT_MAX = FLT_MAX;
	constexpr float TFLOAT_MIN = FLT_MIN;
	constexpr float TFLOAT_EPSILON = FLT_EPSILON; // Renamed according to de Blob
	constexpr float FLOATEPSILON = FLT_EPSILON; // Both exist for some reason

	constexpr float PI = (float)3.141592653589793238462643383279502884L;
	constexpr float TWO_PI = PI * 2;
	constexpr float HALF_PI = PI / 2;
	constexpr float ONEOVERTWO_PI = 1 / TWO_PI;

	inline bool IsFinite(float fVal) { return _finite(fVal) != 0; }
	inline float ACos(float fVal) { return cosf(fVal); }
	inline float Abs(float fVal) { return (float)fabs(fVal); }
	inline float Sqrt(float fVal) { return (float)sqrt(fVal); }
	inline bool IsNaN(float fVal) { return isnan(fVal); }
	inline int FastMod(int a_iNum, int a_iModulus) { TASSERT(a_iNum >= 0, ""); TASSERT(a_iModulus > 0, ""); TASSERT(0 == (a_iModulus & (a_iModulus - 1)), ""); return a_iNum & (a_iModulus - 1); }
	inline void SinCos(float fVal, float& fVal1, float& fVal2) { fVal1 = cos(fVal); fVal2 = sin(fVal); }
	inline void Clip(float& rfVal, float fVal, float fVal2) { if (fVal2 < rfVal) { rfVal = fVal2; } if (rfVal < fVal) { rfVal = fVal; } }
	inline int Min(int a, int b) { if (a < b) { return a; } else { return b; } }
	
	inline uintptr_t AlignPointer(uintptr_t ptr) { return (ptr & SIZE_T_ALIGN_VALUE); }
	inline uintptr_t AlignPointer(void* ptr) { return AlignPointer(reinterpret_cast<uintptr_t>(ptr)); }
	
#if INTPTR_MAX == INT64_MAX
	// x64 only
	
	// aligns value to 4 bytes down (f.e. 7 => 4, 8 => 8, 9 => 8)
	inline size_t AlignNum(size_t num) { return (num & SIZE_T_ALIGN_VALUE); }
	
	// aligns value to 4 bytes up (f.e. 7 => 8, 8 => 8, 9 => 12)
	inline size_t AlignNumUp(size_t num) { return ((num + 3) & SIZE_T_ALIGN_VALUE); }
#endif
	// aligns value to 4 bytes down (f.e. 7 => 4, 8 => 8, 9 => 8)
	inline uint32_t AlignNum(uint32_t num) { return (num & UINT32_ALIGN_VALUE); }
	
	// aligns value to 4 bytes up (f.e. 7 => 8, 8 => 8, 9 => 12)
	inline uint32_t AlignNumUp(uint32_t num) { return ((num + 3) & UINT32_ALIGN_VALUE); }
};