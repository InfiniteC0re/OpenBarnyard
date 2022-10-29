#include "pch.h"
#include "ALevelInformation.h"

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
	int size = PARSEDWORD(m_pData);
	m_pHashTable->Create(size, 0x80, size, size);

	for (size_t i = 0; i < size; i++)
	{

	}

}

bool ALevelInformation::LevelNameHashCompareFunc(void* unk, void* unk2, int unk3)
{
	return Toshi::TStringManager::String8Compare((char*)unk, (char*) unk2, -1) == 0;
}
