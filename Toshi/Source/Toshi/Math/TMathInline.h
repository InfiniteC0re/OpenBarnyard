#pragma once

namespace Toshi
{
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

		constexpr float MAXFLOAT = FLT_MAX;
		constexpr float MINFLOAT = FLT_MIN;
		constexpr float TFLOAT_EPSILON = FLT_EPSILON; // Renamed according to de Blob
		constexpr float FLOATEPSILON = FLT_EPSILON; // Both exist for some reason

		constexpr float PI = 3.1415927f;
		constexpr float TWO_PI = PI * 2;
		constexpr float HALF_PI = PI / 2;
		constexpr float ONEOVER_SQRT_TWO = 0.70710677f;
		constexpr float ONEOVERTWO_PI = 1 / TWO_PI;

		inline TBOOL IsFinite(float fVal) { return _finite(fVal) != 0; }
		inline float Sin(float fVal) { return sinf(fVal); }
		inline float Cos(float fVal) { return cosf(fVal); }
		inline float Tan(float fVal) { return tanf(fVal); }
		inline float ASin(float fVal) { return sinf(fVal); }
		inline float ACos(float fVal) { return acos(fVal); }
		inline float ATan(float fVal) { float fRes = atanf(fVal); TASSERT(TMath::IsFinite(fRes)); return fRes; }
		inline float ATan2(float fVal1, float fVal2) { float fRes = atan2f(fVal1, fVal2); TASSERT(TMath::IsFinite(fRes)); return fRes; }
		inline float Abs(float fVal) { return (float)fabs(fVal); }
		inline constexpr float DegToRad(float fDeg) { return (fDeg / 180.0f) * PI; }

		inline float Sqrt(float a_fX) 
		{
			TASSERT(a_fX != 0.0f);
			float fVal = sqrtf(a_fX);
			TASSERT(IsFinite(fVal));
			return fVal;
		}

		inline float OneOverSqrt(float a_fX)
		{
			return 1.0f / Sqrt(a_fX);
		}

		inline int CeilToInt(float a_fVal) { return int(a_fVal) - uint32_t(0 < uint32_t(a_fVal - uint32_t(a_fVal))); }
		inline int FloorToInt(float a_fVal) { return int(a_fVal) - uint32_t(0x80000000 < uint32_t(a_fVal - uint32_t(a_fVal))); }
		inline TBOOL IsNaN(float fVal) { return isnan(fVal); }
		inline int FastMod(int a_iNum, int a_iModulus) { TASSERT(a_iNum >= 0); TASSERT(a_iModulus > 0); TASSERT(0 == (a_iModulus & (a_iModulus - 1))); return a_iNum & (a_iModulus - 1); }
		inline void SinCos(float fVal, float& a_rSin, float& a_rCos) { a_rSin = sin(fVal); a_rCos = cos(fVal); }
		inline float LERP(float a, float b, float t) { return a + t * (b - a); }
		
		template <typename T>
		inline void Clip(T& rVal, const T& Min, const T& Max) { if (Max < rVal) { rVal = Max; } if (rVal < Min) { rVal = Min; } }
		
		template <typename T>
		inline const T& Min(const T& a, const T& b) { if (a < b) { return a; } else { return b; } }
		
		template <typename T>
		inline const T& Max(const T& a, const T& b) { if (a < b) { return b; } else { return a; } }

		inline uintptr_t AlignPointer(uintptr_t ptr) { return ((ptr + 3) & SIZE_T_ALIGN_VALUE); }
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
}