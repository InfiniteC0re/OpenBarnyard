#pragma once
#include "T2GUIFont.h"
#include "T2GUIFontResource.h"

TOSHI_NAMESPACE_START

class T2GUIFontManager
{
public:
	static constexpr TUINT MAX_NUM_LOADED_FONTS = 1;
	static constexpr TUINT MAX_NUM_FONT_REFS    = 16;

public:
	static TBOOL Open( const TCHAR* a_szFileName );

	static T2GUIFont*         FindFont( const TCHAR* a_szFontName );
	static T2GUIFontRef*      FindFontRef( const TCHAR* a_szFontName );
	static T2GUIFontResource* FindFontResource( const TCHAR* a_szResourceName );

private:
	static T2GUIFontRef*      FindUnusedFontRef();
	static T2GUIFontResource* FindUnusedFontResource();
	static void               CreateFontFromResource( T2GUIFontResource* a_pFontResource );

public:
	inline static T2GUIFontResource s_fontresources[ MAX_NUM_LOADED_FONTS ];
	inline static T2GUIFontRef      s_fontrefs[ MAX_NUM_FONT_REFS ];
};

TOSHI_NAMESPACE_END
