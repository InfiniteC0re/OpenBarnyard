#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi
{
	namespace TMath
	{
		// can be used to align size_t value to 4
		constexpr size_t SIZE_T_ALIGN_VALUE = UINTPTR_MAX - (sizeof(TUINT32) - 1);

		// can be used to align TUINT32 value to 4 
		constexpr TUINT UINT32_ALIGN_VALUE = UINT32_MAX - (sizeof(TUINT32) - 1);

		constexpr TINT8 TINT8_MAX = INT8_MAX;
		constexpr TINT8 TINT8_MIN = INT8_MIN;
		constexpr TUINT8 TUINT8_MAX = UINT8_MAX;

		constexpr TUINT TINT16_MAX = INT16_MAX;
		constexpr TUINT TINT16_MIN = INT16_MIN;
		constexpr TUINT TUINT16_MAX = UINT16_MAX;

		constexpr TINT TINT32_MAX = INT32_MAX;
		constexpr TINT TINT32_MIN = INT32_MIN;
		constexpr TUINT TUINT32_MAX = UINT32_MAX;

		constexpr TUINT16 MAXWCHAR = WCHAR_MAX;

		constexpr TFLOAT MAXFLOAT = FLT_MAX;
		constexpr TFLOAT MINFLOAT = FLT_MIN;
		constexpr TFLOAT TFLOAT_EPSILON = FLT_EPSILON; // Renamed according to de Blob
		constexpr TFLOAT FLOATEPSILON = FLT_EPSILON; // Both exist for some reason

		constexpr TFLOAT PI = 3.1415927f;
		constexpr TFLOAT TWO_PI = PI * 2;
		constexpr TFLOAT HALF_PI = PI / 2;
		constexpr TFLOAT ONEOVER_SQRT_TWO = 0.70710677f;
		constexpr TFLOAT ONEOVERTWO_PI = 1 / TWO_PI;

		inline TBOOL IsFinite(TFLOAT fVal) { return _finite(fVal) != 0; }
		inline TFLOAT Sin(TFLOAT fVal) { return sinf(fVal); }
		inline TFLOAT Cos(TFLOAT fVal) { return cosf(fVal); }
		inline TFLOAT Tan(TFLOAT fVal) { return tanf(fVal); }
		inline TFLOAT ASin(TFLOAT fVal) { return sinf(fVal); }
		inline TFLOAT ACos(TFLOAT fVal) { return acos(fVal); }
		inline TFLOAT ATan(TFLOAT fVal) { TFLOAT fRes = atanf(fVal); TASSERT(TMath::IsFinite(fRes)); return fRes; }
		inline TFLOAT ATan2(TFLOAT fVal1, TFLOAT fVal2) { TFLOAT fRes = atan2f(fVal1, fVal2); TASSERT(TMath::IsFinite(fRes)); return fRes; }
		inline TFLOAT Abs(TFLOAT fVal) { return (TFLOAT)fabs(fVal); }
		inline constexpr TFLOAT DegToRad(TFLOAT fDeg) { return (fDeg / 180.0f) * PI; }
		
		inline constexpr TUINT IntLog2(TUINT32 a_uiValue)
		{
			return std::bit_width(a_uiValue) - 1;
		}

		inline constexpr TUINT RoundToNextPowerOfTwo(TUINT a_uiValue)
		{
			a_uiValue--;
			a_uiValue |= a_uiValue >> 1;
			a_uiValue |= a_uiValue >> 2;
			a_uiValue |= a_uiValue >> 4;
			a_uiValue |= a_uiValue >> 8;
			a_uiValue |= a_uiValue >> 16;
			a_uiValue++;

			return a_uiValue;
		}

		inline TFLOAT Sqrt(TFLOAT a_fX) 
		{
			TASSERT(a_fX != 0.0f);
			TFLOAT fVal = sqrtf(a_fX);
			TASSERT(IsFinite(fVal));
			return fVal;
		}

		inline TFLOAT OneOverSqrt(TFLOAT a_fX)
		{
			return 1.0f / Sqrt(a_fX);
		}

		inline TINT CeilToInt(TFLOAT a_fVal) { return TINT(a_fVal) - TUINT32(0 < TUINT32(a_fVal - TUINT32(a_fVal))); }
		inline TINT FloorToInt(TFLOAT a_fVal) { return TINT(a_fVal) - TUINT32(0x80000000 < TUINT32(a_fVal - TUINT32(a_fVal))); }
		inline TBOOL IsNaN(TFLOAT fVal) { return isnan(fVal); }
		inline TINT FastMod(TINT a_iNum, TINT a_iModulus) { TASSERT(a_iNum >= 0); TASSERT(a_iModulus > 0); TASSERT(0 == (a_iModulus & (a_iModulus - 1))); return a_iNum & (a_iModulus - 1); }
		inline void SinCos(TFLOAT fVal, TFLOAT& a_rSin, TFLOAT& a_rCos) { a_rSin = sin(fVal); a_rCos = cos(fVal); }
		inline TFLOAT LERP(TFLOAT a, TFLOAT b, TFLOAT t) { return a + t * (b - a); }
		
		template <typename T>
		inline void Clip(T& rVal, const T& Min, const T& Max) { if (Max < rVal) { rVal = Max; } if (rVal < Min) { rVal = Min; } }
		
		template <typename T>
		inline const T& Min(const T& a, const T& b) { if (a < b) { return a; } else { return b; } }
		
		template <typename T>
		inline const T& Max(const T& a, const T& b) { if (a < b) { return b; } else { return a; } }

		inline uintptr_t AlignPointer(uintptr_t ptr) { return ((ptr + 3) & SIZE_T_ALIGN_VALUE); }
		inline uintptr_t AlignPointer(void* ptr) { return AlignPointer(TREINTERPRETCAST(uintptr_t, ptr)); }

#if INTPTR_MAX == INT64_MAX
		// x64 only

		// aligns value to 4 bytes down (f.e. 7 => 4, 8 => 8, 9 => 8)
		inline size_t AlignNum(size_t num) { return (num & SIZE_T_ALIGN_VALUE); }

		// aligns value to 4 bytes up (f.e. 7 => 8, 8 => 8, 9 => 12)
		inline size_t AlignNumUp(size_t num) { return ((num + 3) & SIZE_T_ALIGN_VALUE); }
#endif
		// aligns value to 4 bytes down (f.e. 7 => 4, 8 => 8, 9 => 8)
		inline TUINT32 AlignNum(TUINT32 num) { return (num & UINT32_ALIGN_VALUE); }

		// aligns value to 4 bytes up (f.e. 7 => 8, 8 => 8, 9 => 12)
		inline TUINT32 AlignNumUp(TUINT32 num) { return ((num + 3) & UINT32_ALIGN_VALUE); }
	};

	constexpr TBOOL TIsAlignedAddress(const void* a_pPointer, TUINT32 a_uiAlignment)
	{
		return ISZERO(uintptr_t(a_pPointer) & (a_uiAlignment - 1));
	}
}