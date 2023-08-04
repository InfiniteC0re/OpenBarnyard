#include "pch.h"
#include "AEntityFactory.h"

void AEntityFactory::CreateEntity(const char* a_szEntityName, int unk)
{
	Toshi::TString8 str = "data/entities/";
	str += a_szEntityName;
	GetEntityTypeData(a_szEntityName);
	Toshi::TClass* t;
	t->CreateObject();
}

AEntityFactory::AEntityTypeData* AEntityFactory::GetEntityTypeData(const char* a_szEntityName)
{
	for (size_t i = 0; i < m_sEntityTypeDatas.Size(); i++)
	{
		if (Toshi::T2String8::CompareNoCase(m_sEntityTypeDatas[i]->m_szEntityName, a_szEntityName)) return m_sEntityTypeDatas[i];
	}
	return AddPrecachedTypeData(a_szEntityName);
}

AEntityFactory::AEntityTypeData* AEntityFactory::AddPrecachedTypeData(const char* a_szEntityName)
{
	return TNULL;
}
