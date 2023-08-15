#include "pch.h"
#include "AResourceViewLoader.h"
#include "APropertiesResourceView.h"

struct AResourceInitialiserMap
{
	const char* SymbolName;
	AResourceView* (*CreateView)(AResourceFilePtr a_pResourceFile);
};

static constexpr AResourceInitialiserMap oInitialiser[] =
{
	{
		"Main",
		[](AResourceFilePtr a_pResourceFile) -> AResourceView* {
			return new APropertiesResourceView(a_pResourceFile);
		}
	}
};

static constexpr size_t oInitialiserSize = sizeof(oInitialiser) / sizeof(*oInitialiser);

void AResourceViewLoader::Update(float a_fDeltaTime)
{
	for (auto it = m_Resources.Begin(); it != m_Resources.End();)
	{
		if (it->IsSetToBeDestroyed())
		{
			auto pNext = it->Next();
			delete it;
			it = pNext;
		}
		else
		{
			it++;
		}
	}
}

TBOOL AResourceViewLoader::CreateFile(const char* a_szFileName, AResourceFilePtr* a_pResourceFile)
{
	AResourceFilePtr pResFile = AResourceFile::Create();
	TBOOL bLoadedFile = pResFile->LoadFile(a_szFileName);

	if (bLoadedFile)
	{
		auto& trb = pResFile->GetTRB();
		auto pSymbolTable = trb.GetSymbolTable();

		for (int i = 0; i < pSymbolTable->m_i32SymbCount; i++)
		{
			auto szSymbol = trb.GetSymbolName(i);

			for (size_t k = 0; k < oInitialiserSize; k++)
			{
				if (Toshi::TStringManager::String8Compare(oInitialiser[k].SymbolName, szSymbol) == 0)
				{
					AResourceView* pResourceView = oInitialiser[k].CreateView(pResFile);

					if (pResourceView != TNULL)
					{
						m_Resources.PushBack(pResourceView);
					}
				}
			}
		}

		if (a_pResourceFile)
			*a_pResourceFile = pResFile;
	}

	return bLoadedFile;
}
