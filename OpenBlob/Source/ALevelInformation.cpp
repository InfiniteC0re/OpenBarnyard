#include "pch.h"
#include "ALevelInformation.h"


Toshi::THashTable::t_ItemCompareFunc ALevelInformation::LevelNameHashCompareFunc = [](void* unk, void* unk2, int unk3)
{
	return Toshi::TStringManager::String8Compare((char*)unk, (char*)unk2, -1) == 0;
};

void ALevelInformation::Create(const char* a_trbName)
{
	m_pData = TNULL;
	Close();
	if (Load(a_trbName) == ERROR_OK)
	{
		m_pData = reinterpret_cast<uint8_t*>(GetSymbolAddress("LevelsMain"));
	}
	MakeStringToIDHashTable();
}

void ALevelInformation::MakeStringToIDHashTable()
{
	int levelCount = PARSEDWORD(m_pData);
	m_pHashTable->Create(levelCount, 0x80, levelCount, levelCount);
	m_pHashTable->SetItemCompareFunction(LevelNameHashCompareFunc);
	LevelInformations* curLevel = reinterpret_cast<LevelInformations*>(m_pData);
	for (size_t i = 0; i < levelCount; i++)
	{
		char toAddLevelName[0x80];
		char* curLevelName = curLevel->Strings[i].levelName;
		Toshi::TStringManager::String8Copy(toAddLevelName, curLevelName, 0x80);
		m_pHashTable->Insert(toAddLevelName);
	}

}
