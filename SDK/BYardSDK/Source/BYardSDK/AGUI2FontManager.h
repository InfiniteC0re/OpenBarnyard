#pragma once
#include "AGUI2Font.h"
#include "AGUI2FontResource.h"

class AGUI2FontManager
{
public:
	static constexpr TUINT MAX_NUM_LOADED_FONTS = 1;
	static constexpr TUINT MAX_NUM_FONT_REFS    = 16;

public:
	static TBOOL Open( const char* a_szFileName );

	static AGUI2Font*         FindFont( const char* a_szFontName );
	static AGUI2FontRef*      FindFontRef( const char* a_szFontName );
	static AGUI2FontResource* FindFontResource( const char* a_szResourceName );

private:
	static AGUI2FontRef*      FindUnusedFontRef();
	static AGUI2FontResource* FindUnusedFontResource();
	static void               CreateFontFromResource( AGUI2FontResource* a_pFontResource );

public:
	inline static AGUI2FontResource* s_fontresources = (AGUI2FontResource*)0x007cec98;
	inline static AGUI2FontRef*      s_fontrefs      = (AGUI2FontRef*)0x007cede0;
};
