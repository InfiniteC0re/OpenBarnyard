#include "pch.h"
#include "AGUI2FontManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL AGUI2FontManager::Open(const TCHAR* a_szFileName)
{
	auto pResource = FindFontResource(a_szFileName);

	if (pResource)
	{
		pResource->uiRefCount++;
		return TTRUE;
	}

	pResource = FindUnusedFontResource();
	TASSERT(pResource != TNULL);

	if (!pResource)
	{
		return TFALSE;
	}

	if (pResource->oTRB.Load(a_szFileName) != TTRB::ERROR_OK)
	{
		return TFALSE;
	}

	pResource->szFileName = new TCHAR[TStringManager::String8Length(a_szFileName) + 1];
	TStringManager::String8Copy(pResource->szFileName, a_szFileName);

	pResource->uiRefCount++;
	CreateFontFromResource(pResource);

	return TTRUE;
}

AGUI2Font* AGUI2FontManager::FindFont(const TCHAR* a_szFontName)
{
	auto pRef = FindFontRef(a_szFontName);

	if (pRef)
	{
		pRef->uiNumRefs += 1;
		return pRef->pFont;
	}

	return TNULL;
}

AGUI2FontRef* AGUI2FontManager::FindFontRef(const TCHAR* a_szFontName)
{
	for (TUINT i = 0; i < MAX_NUM_FONT_REFS; i++)
	{
		if (s_fontrefs[i].pFontResource != TNULL)
		{
			if (TStringManager::String8CompareNoCase(s_fontrefs[i].szName, a_szFontName) == 0)
			{
				return &s_fontrefs[i];
			}
		}
	}

	return TNULL;
}

AGUI2FontResource* AGUI2FontManager::FindFontResource(const TCHAR* a_szResourceName)
{
	for (TUINT i = 0; i < MAX_NUM_LOADED_FONTS; i++)
	{
		if (s_fontresources[i].uiRefCount > 0)
		{
			if (TStringManager::String8CompareNoCase(s_fontresources[i].szFileName, a_szResourceName) == 0)
			{
				return &s_fontresources[i];
			}
		}
	}

	return TNULL;
}

AGUI2FontRef* AGUI2FontManager::FindUnusedFontRef()
{
	for (TUINT i = 0; i < MAX_NUM_FONT_REFS; i++)
	{
		if (s_fontrefs[i].pFontResource == TNULL)
		{
			return &s_fontrefs[i];
		}
	}

	return TNULL;
}

AGUI2FontResource* AGUI2FontManager::FindUnusedFontResource()
{
	for (TUINT i = 0; i < MAX_NUM_LOADED_FONTS; i++)
	{
		if (s_fontresources[i].uiRefCount == 0)
		{
			return &s_fontresources[i];
		}
	}

	return TNULL;
}

void AGUI2FontManager::CreateFontFromResource(AGUI2FontResource* a_pFontResource)
{
	auto& trb = a_pFontResource->oTRB;

	for (TINT i = 0; i < trb.GetNumSymbols(); i++)
	{
		auto pSymbol = trb.GetSymbol(i);
		auto pSymbolName = trb.GetSymbolName(pSymbol);

		auto pFontRef = FindFontRef(pSymbolName);

		if (pFontRef == TNULL)
		{
			pFontRef = FindUnusedFontRef();
			TASSERT(pFontRef != TNULL);

			if (pFontRef == TNULL) return;

			pFontRef->szName = new TCHAR[TStringManager::String8Length(pSymbolName) + 1];
			TStringManager::String8Copy(pFontRef->szName, pSymbolName);

			auto pFontDef = trb.CastSymbol<AGUI2FontDef>(pSymbolName);
			pFontRef->pFont = new AGUI2Font();
			pFontRef->pFont->Create(pFontDef);
			pFontRef->pFontResource = a_pFontResource;
			pFontRef->Unk1 = TNULL;
		}
	}
}
