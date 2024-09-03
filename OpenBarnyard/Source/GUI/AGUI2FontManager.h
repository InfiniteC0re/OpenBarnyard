#pragma once
#include "AGUI2Font.h"
#include "AGUI2FontResource.h"

#define AGUI2FONT_PRIMARY   "Rekord26"
#define AGUI2FONT_SECONDARY "Rekord18"

class AGUI2FontManager
{
public:
	static constexpr TUINT MAX_NUM_LOADED_FONTS = 1;
	static constexpr TUINT MAX_NUM_FONT_REFS    = 16;

public:
	static TBOOL Open( const TCHAR* a_szFileName );

	static AGUI2Font*         FindFont( const TCHAR* a_szFontName );
	static AGUI2FontRef*      FindFontRef( const TCHAR* a_szFontName );
	static AGUI2FontResource* FindFontResource( const TCHAR* a_szResourceName );

private:
	static AGUI2FontRef*      FindUnusedFontRef();
	static AGUI2FontResource* FindUnusedFontResource();
	static void               CreateFontFromResource( AGUI2FontResource* a_pFontResource );

public:
	inline static AGUI2FontResource s_fontresources[ MAX_NUM_LOADED_FONTS ];
	inline static AGUI2FontRef      s_fontrefs[ MAX_NUM_FONT_REFS ];
};
