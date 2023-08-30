#include "pch.h"
#include "AGUI2.h"

TOSHI_NAMESPACE_USING

AGUI2TextureSection* AGUI2::GetTextureSection(const char* a_szSectionName)
{
	auto pDefblock = AGUI2TextureSectionManager::g_defblock;
	auto pSections = AGUI2TextureSectionManager::g_sections;

	for (size_t i = 0; i < pDefblock->m_uiNumSections; i++)
	{
		auto pSection = &pSections[i];

		if (TStringManager::String8Compare(pSection->GetName(), a_szSectionName) == 0)
			return pSection;
	}

	return TNULL;
}
