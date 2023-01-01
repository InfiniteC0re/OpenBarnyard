#include "ToshiPCH.h"
#include "TModel.h"

bool Toshi::TModel::LoadTMD(const char* a_tmdPath)
{
	auto file = TFile::Create(a_tmdPath, Toshi::TFile::OpenFlags_Read);

	Header header;

	if (file != TNULL)
	{
		file->Read(&header, sizeof(Header));

		if (header.m_magic == TMAKEFOUR("TMDL"))
		{

		}

		file->Destroy();
	}

	return false;
}

void Toshi::TModel::CreateResource(const char* name)
{
	T2Resource::CreateResource(name, this, TNULL, this);
	
}

bool Toshi::TModel::LoadTRBTMD(const char*)
{
	const char* symbol = TranslateSymbolName("SkeletonHeader");
	m_pTRB->GetSymbolAddress(symbol);
	symbol = TranslateSymbolName("Skeleton");
	m_pTRB->GetSymbolAddress(symbol);
	symbol = TranslateSymbolName("Collision");
	m_pTRB->GetSymbolAddress(symbol);
	return false;
}

bool Toshi::TModel::LoadTrb()
{
	return false;
}

const char* Toshi::TModel::TranslateSymbolName(const char* a_symbolName)
{
	if (sm_SymbolNamePrefix != TNULL)
	{
		T2String8::Format(s_TranslatedSymbol, sizeof(s_TranslatedSymbol), "%s_%s", sm_SymbolNamePrefix);
		a_symbolName = s_TranslatedSymbol;
	}
	return a_symbolName;
}
