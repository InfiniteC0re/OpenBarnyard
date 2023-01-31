#include "ToshiPCH.h"
#include "T2GUIFont.h"

void Toshi::T2GUIFont::SetRegion()
{
	if (TRegion::GetRegion() == 4)
	{
		s_CanBreakBetweenChars = CanBreakBetweenChars_Japanese;
	}
	else
	{
		s_CanBreakBetweenChars = CanBreakBetweenChars_Default;
	}
}
