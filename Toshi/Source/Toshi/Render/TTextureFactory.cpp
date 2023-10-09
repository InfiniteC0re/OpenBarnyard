#include "ToshiPCH.h"
#include "TTextureFactory.h"

namespace Toshi {

	TTextureFactory::NameEntry::NameEntry(const char* a_szName, TTexture* a_pTexture)
	{
		auto uiNameLen = TStringManager::String8Length(a_szName);
		TASSERT(uiNameLen < MAX_TEXTURENAMELEN);

		TStringManager::String8Copy(m_szName, a_szName, MAX_TEXTURENAMELEN);
	}

	TTextureFactory::NameEntry::~NameEntry()
	{
		TASSERT(TFALSE == IsLinked())
	}

	TTextureFactory::~TTextureFactory()
	{
		for (TUINT32 i = 0; i < NUM_LISTS; i++)
		{
			m_aLists[i].DeleteAll();
		}
	}

	void TTextureFactory::Dump() const
	{
		TIMPLEMENT();

		for (TUINT32 i = 0; i < NUM_LISTS; i++)
		{
			for (auto it = m_aLists[i].Begin(); it != m_aLists[i].End(); it++)
			{

			}
		}
	}

}
