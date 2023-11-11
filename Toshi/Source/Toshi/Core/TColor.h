#pragma once
#include "Toshi/Typedefs.h"

namespace Toshi {

	struct TColor
	{
		union
		{
			struct
			{
				TUINT8 B;
				TUINT8 G;
				TUINT8 R;
				TUINT8 A;
			};

			TUINT32 Value32;
		};

		__forceinline constexpr TColor() { }

		__forceinline constexpr TColor(TUINT32 a_uiColor) :
			Value32(a_uiColor) { }

		__forceinline constexpr TColor(TUINT8 a_uiR, TUINT8 a_uiG, TUINT8 a_uiB, TUINT8 a_uiA = 255)
			: R(a_uiR), G(a_uiG), B(a_uiB), A(a_uiA) { }

		__forceinline TColor& operator=(TUINT32 a_uiColor)
		{
			Value32 = a_uiColor;
		}
	};

	static_assert(sizeof(TColor) == 4);

}