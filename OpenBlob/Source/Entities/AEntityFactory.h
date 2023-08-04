#pragma once
#include "AStack.h"

class AEntityFactory
{

	struct AEntityTypeData
	{
		int unk;
		char* m_szEntityName;
	};

	void CreateEntity(const char* a_szEntityName, int unk);

	AEntityTypeData* GetEntityTypeData(const char* a_szEntityName);

	AEntityTypeData* AddPrecachedTypeData(const char* a_szEntityName);

	AStack<AEntityTypeData*, 128> m_sEntityTypeDatas;

};