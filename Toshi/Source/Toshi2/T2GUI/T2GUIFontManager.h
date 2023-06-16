#pragma once
#include "T2GUIFont.h"

namespace Toshi
{
	static int* FindFontRef(const char* a_font);
	static int* FindFreeFontRef();

	class T2GUIFontDef2
	{
		char* m_font;

		TBOOL Init();
	};

	class T2GUIFontManager
	{

		TBOOL Open()
		{

		}
	};

}