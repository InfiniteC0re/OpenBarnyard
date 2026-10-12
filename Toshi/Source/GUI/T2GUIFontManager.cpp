#include "ToshiPCH.h"
#include "T2GUIFontManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: the Wii version (804a1424) has a second argument: TTRUE loads "<font directory><a_szFileName>.trb",
// the font directory being "Data/XUI/Fonts/<language>/" (set by 804a13f0 from AGUI2::OnCreate), and it
// keeps 8 font resources. It's only used for the paged XUI fonts, which aren't supported.
// $Barnyard: FUNCTION 006c4410
TBOOL T2GUIFontManager::Open( const TCHAR* a_szFileName )
{
	auto pResource = FindFontResource( a_szFileName );

	if ( pResource )
	{
		pResource->uiRefCount++;
		return TTRUE;
	}

	pResource = FindUnusedFontResource();
	TASSERT( pResource != TNULL );

	if ( !pResource )
	{
		return TFALSE;
	}

	if ( pResource->oTRB.Load( a_szFileName ) != TTRB::ERROR_OK )
	{
		return TFALSE;
	}

	pResource->szFileName = new TCHAR[ TStringManager::String8Length( a_szFileName ) + 1 ];
	TStringManager::String8Copy( pResource->szFileName, a_szFileName );

	pResource->uiRefCount++;
	CreateFontFromResource( pResource );

	return TTRUE;
}

// $Barnyard: FUNCTION 006c44d0
T2GUIFont* T2GUIFontManager::FindFont( const TCHAR* a_szFontName )
{
	auto pRef = FindFontRef( a_szFontName );

	if ( pRef )
	{
		pRef->uiNumRefs += 1;
		return pRef->pFont;
	}

	return TNULL;
}

// Decrements the reference count of the font that FindFont returned
// $Barnyard: FUNCTION 006c44f0
void T2GUIFontManager::ReleaseFont( T2GUIFont* a_pFont )
{
	if ( !a_pFont )
		return;

	for ( TUINT i = 0; i < MAX_NUM_FONT_REFS; i++ )
	{
		if ( s_fontrefs[ i ].pFontResource != TNULL && s_fontrefs[ i ].pFont == a_pFont )
		{
			s_fontrefs[ i ].uiNumRefs -= 1;
			return;
		}
	}

	// NOTE: the original decrements through a null ref here (writes to address 0x10)
	TASSERT( TFALSE && "T2GUIFontManager::ReleaseFont: unknown font" );
}

// Releases the file name and the fonts of a font resource and unloads it once it's not referenced anymore
// $Barnyard: FUNCTION 006c45a0
void T2GUIFontManager::Close( const TCHAR* a_szFileName )
{
	T2GUIFontResource* pResource = FindFontResource( a_szFileName );

	if ( pResource && pResource->uiRefCount-- == 1 )
	{
		DestroyFontRefs( pResource );
		pResource->oTRB.Close();
		delete[] pResource->szFileName;
	}
}

// $Barnyard: FUNCTION 006c4540
void T2GUIFontManager::DestroyFontRefs( T2GUIFontResource* a_pFontResource )
{
	for ( TUINT i = 0; i < MAX_NUM_FONT_REFS; i++ )
	{
		T2GUIFontRef& rFontRef = s_fontrefs[ i ];

		if ( rFontRef.pFontResource == a_pFontResource )
		{
			if ( rFontRef.pUnk1 )
			{
				delete rFontRef.pUnk1;
				rFontRef.pUnk1 = TNULL;
			}

			if ( TINT( rFontRef.uiNumRefs ) > 0 )
				rFontRef.uiNumRefs = 0;

			delete[] rFontRef.szName;
			delete rFontRef.pFont;

			rFontRef.pFontResource = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 006c42a0
T2GUIFontRef* T2GUIFontManager::FindFontRef( const TCHAR* a_szFontName )
{
	for ( TUINT i = 0; i < MAX_NUM_FONT_REFS; i++ )
	{
		if ( s_fontrefs[ i ].pFontResource != TNULL )
		{
			if ( TStringManager::String8CompareNoCase( s_fontrefs[ i ].szName, a_szFontName ) == 0 )
			{
				return &s_fontrefs[ i ];
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006c4240
T2GUIFontResource* T2GUIFontManager::FindFontResource( const TCHAR* a_szResourceName )
{
	for ( TUINT i = 0; i < MAX_NUM_LOADED_FONTS; i++ )
	{
		if ( s_fontresources[ i ].uiRefCount > 0 )
		{
			if ( TStringManager::String8CompareNoCase( s_fontresources[ i ].szFileName, a_szResourceName ) == 0 )
			{
				return &s_fontresources[ i ];
			}
		}
	}

	return TNULL;
}

T2GUIFontRef* T2GUIFontManager::FindUnusedFontRef()
{
	for ( TUINT i = 0; i < MAX_NUM_FONT_REFS; i++ )
	{
		if ( s_fontrefs[ i ].pFontResource == TNULL )
		{
			return &s_fontrefs[ i ];
		}
	}

	return TNULL;
}

T2GUIFontResource* T2GUIFontManager::FindUnusedFontResource()
{
	for ( TUINT i = 0; i < MAX_NUM_LOADED_FONTS; i++ )
	{
		if ( s_fontresources[ i ].uiRefCount == 0 )
		{
			return &s_fontresources[ i ];
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006c42f0
void T2GUIFontManager::CreateFontFromResource( T2GUIFontResource* a_pFontResource )
{
	auto& trb = a_pFontResource->oTRB;

	for ( TINT i = 0; i < trb.GetNumSymbols(); i++ )
	{
		auto pSymbol     = trb.GetSymbol( i );
		auto pSymbolName = trb.GetSymbolName( pSymbol );

		auto pFontRef = FindFontRef( pSymbolName );

		if ( pFontRef == TNULL )
		{
			pFontRef = FindUnusedFontRef();
			TASSERT( pFontRef != TNULL );

			if ( pFontRef == TNULL ) return;

			pFontRef->szName = new TCHAR[ TStringManager::String8Length( pSymbolName ) + 1 ];
			TStringManager::String8Copy( pFontRef->szName, pSymbolName );

			auto pFontDef   = trb.CastSymbol<T2GUIFontDef>( pSymbolName );
			pFontRef->pFont = new T2GUIFont();
			pFontRef->pFont->Create( pFontDef );
			pFontRef->pFontResource = a_pFontResource;
			pFontRef->pUnk1         = TNULL;
		}
	}
}
