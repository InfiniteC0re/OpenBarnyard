#pragma once
#include "SDK_T2GUIFont.h"

#include <GUI/T2GUIFontResource.h>

class SDK_T2GUIFontManager
{
public:
	static constexpr TUINT MAX_NUM_LOADED_FONTS = 1;
	static constexpr TUINT MAX_NUM_FONT_REFS    = 16;

public:
	static TBOOL Open( const char* a_szFileName );

	static SDK_T2GUIFont*            FindFont( const char* a_szFontName );
	static SDK_T2GUIFontRef*         FindFontRef( const char* a_szFontName );
	static Toshi::T2GUIFontResource* FindFontResource( const char* a_szResourceName );

private:
	static SDK_T2GUIFontRef*         FindUnusedFontRef();
	static Toshi::T2GUIFontResource* FindUnusedFontResource();
	static void                      CreateFontFromResource( Toshi::T2GUIFontResource* a_pFontResource );

public:
	inline static Toshi::T2GUIFontResource* s_fontresources = (Toshi::T2GUIFontResource*)0x007cec98;
	inline static SDK_T2GUIFontRef*         s_fontrefs      = (SDK_T2GUIFontRef*)0x007cede0;
};
