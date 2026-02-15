#include "ToshiPCH.h"
#include "T2GUIFontManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

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
			pFontRef->Unk1          = TNULL;
		}
	}
}
