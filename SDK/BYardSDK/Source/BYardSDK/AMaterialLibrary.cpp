#include "pch.h"
#include "AMaterialLibrary.h"
#include "AMaterialLibraryManager.h"

#include <BYardSDK/SDKHooks.h>

AMaterialLibrary::AMaterialLibrary()
{
	m_iNumTextures = 0;
}

TBOOL AMaterialLibrary::LoadTTLData(void* a_pTTLData)
{
	return CALL_THIS(0x00615d20, AMaterialLibrary*, TBOOL, this, void*, a_pTTLData);
}

TBOOL AMaterialLibrary::LoadTTLFile(const char* a_szFilePath)
{
	return CALL_THIS(0x00616000, AMaterialLibrary*, TBOOL, this, const char*, a_szFilePath);
}

TBOOL AMaterialLibrary::LoadTRBFile(Toshi::TTRB* a_pTRB)
{
	return CALL_THIS(0x00616040, AMaterialLibrary*, TBOOL, this, Toshi::TTRB*, a_pTRB);
}

void AMaterialLibrary::Destroy()
{
	CALL_THIS(0x00615900, AMaterialLibrary*, void, this);
}

void AMaterialLibrary::DestroyTextures()
{
	CALL_THIS(0x00615f60, AMaterialLibrary*, void, this);
}

TINT AMaterialLibrary::FindTextureIndex(const char* a_szTextureName)
{
	for (TINT i = 0; i < m_iNumTextures; i++)
	{
		if (Toshi::TStringManager::String8CompareNoCase(m_pTextures[i].Name, a_szTextureName) == 0)
		{
			return i;
		}
	}

	return -1;
}
