#pragma once

#include "Toshi/Utils/TRegion.h"


namespace Toshi
{
	using t_CanBreakBetweenChars = bool (*)(wchar_t, wchar_t);

	bool CanBreakBetweenChars_Default(wchar_t, wchar_t)
	{
		TIMPLEMENT();
		return false;
	}

	bool CanBreakBetweenChars_Japanese(wchar_t, wchar_t)
	{
		TIMPLEMENT();
		return false;
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