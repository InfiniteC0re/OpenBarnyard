#include "pch.h"
#include "AGUI2TextureSection.h"

TOSHI_NAMESPACE_USING

TBOOL AGUI2TextureSectionManager::Open(const char* a_szFileName, Toshi::TTRB* a_pTRB)
{
	if (a_pTRB == TNULL)
	{
		g_providedtrb = TFALSE;
		g_trb = new TTRB();

		if (g_trb->Load(a_szFileName) != TTRB::ERROR_OK)
			return TFALSE;

		g_sections = TSTATICCAST(AGUI2TextureSection**, g_trb->GetSymbolAddress("texturesections"));
	}
	else
	{
		g_trb = a_pTRB;
		g_providedtrb = TTRUE;

		auto iFilePathLength = TStringManager::String8Length(a_szFileName);
		auto iFileNamePos = iFilePathLength - 1;

		while (a_szFileName[iFileNamePos] != '\\' && a_szFileName[iFileNamePos] != '/')
			iFileNamePos--;

		iFileNamePos++;
		char symbolName[132] = { '\0' };

		auto iFileNameLength = iFilePathLength - iFileNamePos - 4;
		TStringManager::String8Copy(symbolName, a_szFileName + iFileNamePos, iFileNameLength);
		symbolName[iFileNameLength] = '_';
		symbolName[iFileNameLength + 1] = '\0';
		TStringManager::String8ToLowerCase(symbolName);
		TStringManager::String8Copy(symbolName + iFileNameLength + 1, "texturesections");
		
		return TTRUE;
	}

	g_defblock = *g_sections;
	CreateMaterials();

	return TTRUE;
}

void AGUI2TextureSectionManager::Close()
{
	if (g_trb)
	{
		DestroyMaterials();
		g_sections = TNULL;
		g_defblock = TNULL;

		if (!g_providedtrb)
			delete g_trb;

		g_providedtrb = TFALSE;
		g_trb = TNULL;
	}
}

void AGUI2TextureSectionManager::CreateMaterials()
{
	TIMPLEMENT();
}

void AGUI2TextureSectionManager::DestroyMaterials()
{
	TIMPLEMENT();
}
