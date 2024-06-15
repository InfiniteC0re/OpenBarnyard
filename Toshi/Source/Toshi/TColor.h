#pragma once
#include "Toshi/Typedefs.h"

#define TCOLOR(R, G, B)          ((((R & 0xff) << 8 | G & 0xff) << 8 | B & 0xff) | 0xFF000000)
#define TCOLOR_ALPHA(R, G, B, A) ((((R & 0xff) << 8 | G & 0xff) << 8 | B & 0xff) | (TUINT32(A) << 24))
#define TCOLOR_GET_ALPHA(COLOR)  ((COLOR >> 24) & 0xff)
#define TCOLOR_GET_RED(COLOR)    ((COLOR >> 16) & 0xff)
#define TCOLOR_GET_GREEN(COLOR)  ((COLOR >> 8) & 0xff)
#define TCOLOR_GET_BLUE(COLOR)   (COLOR & 0xff)
#define TCOLOR_GET_A(COLOR)      TCOLOR_GET_ALPHA(COLOR)
#define TCOLOR_GET_R(COLOR)      TCOLOR_GET_RED(COLOR)
#define TCOLOR_GET_G(COLOR)      TCOLOR_GET_GREEN(COLOR)
#define TCOLOR_GET_B(COLOR)      TCOLOR_GET_BLUE(COLOR)

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

		TColor() = default;

		__forceinline constexpr TColor(TUINT32 a_uiColor) :
			Value32(a_uiColor) { }

		__forceinline constexpr TColor(TUINT8 a_uiR, TUINT8 a_uiG, TUINT8 a_uiB, TUINT8 a_uiA = 255)
			: R(a_uiR), G(a_uiG), B(a_uiB), A(a_uiA) { }

		__forceinline TColor& operator=(TUINT32 a_uiColor)
		{
			Value32 = a_uiColor;
		}
	};

	TSTATICASSERT(sizeof(TColor) == 4);

}