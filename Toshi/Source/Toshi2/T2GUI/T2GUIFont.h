#pragma once
#include "Toshi/Utils/TRegion.h"

namespace Toshi
{
	typedef TBOOL(*t_CanBreakBetweenChars)(wchar_t, wchar_t);

	inline TBOOL CanBreakBetweenChars_Default(wchar_t, wchar_t)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	inline TBOOL CanBreakBetweenChars_Japanese(wchar_t, wchar_t)
	{
		TIMPLEMENT();
		return TFALSE;
	}

	static t_CanBreakBetweenChars s_CanBreakBetweenChars = CanBreakBetweenChars_Default;

	class T2GUIFont
	{
		T2GUIFont()
		{

		}


		void SetRegion();
	};

}