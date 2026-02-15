#include "pch.h"
#include "SDK_T2GUIFontManager.h"

TOSHI_NAMESPACE_USING

TBOOL SDK_T2GUIFontManager::Open( const char* a_szFileName )
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

	pResource->szFileName = new char[ TStringManager::String8Length( a_szFileName ) + 1 ];
	TStringManager::String8Copy( pResource->szFileName, a_szFileName );

	pResource->uiRefCount++;
	CreateFontFromResource( pResource );

	return TTRUE;
}

SDK_T2GUIFont* SDK_T2GUIFontManager::FindFont( const char* a_szFontName )
{
	auto pRef = FindFontRef( a_szFontName );

	if ( pRef )
	{
		pRef->uiNumRefs += 1;
		return pRef->pFont;
	}

	return TNULL;
}

SDK_T2GUIFontRef* SDK_T2GUIFontManager::FindFontRef( const char* a_szFontName )
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

Toshi::T2GUIFontResource* SDK_T2GUIFontManager::FindFontResource( const char* a_szResourceName )
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

SDK_T2GUIFontRef* SDK_T2GUIFontManager::FindUnusedFontRef()
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

Toshi::T2GUIFontResource* SDK_T2GUIFontManager::FindUnusedFontResource()
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

void SDK_T2GUIFontManager::CreateFontFromResource( Toshi::T2GUIFontResource* a_pFontResource )
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

			pFontRef->szName = new char[ TStringManager::String8Length( pSymbolName ) + 1 ];
			TStringManager::String8Copy( pFontRef->szName, pSymbolName );

			auto pFontDef   = trb.CastSymbol<SDK_T2GUIFontDef>( pSymbolName );
			pFontRef->pFont = new SDK_T2GUIFont();
			pFontRef->pFont->Create( pFontDef );
			pFontRef->pFontResource = a_pFontResource;
			pFontRef->Unk1          = TNULL;
		}
	}
}
