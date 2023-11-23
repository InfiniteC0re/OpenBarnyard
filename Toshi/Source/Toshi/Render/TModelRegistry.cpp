#include "ToshiPCH.h"
#include "TModelRegistry.h"
#include "TRenderInterface.h"

namespace Toshi {

	void TModelRegistry::Initialise()
	{
		TASSERT(ms_pEntries.GetArray() == TNULL);
		ms_pEntries.Create(MAX_NUM_MODELS);
	}

	void TModelRegistry::Uninitialise()
	{
		TASSERT(ms_pEntries.GetArray() != TNULL);
		ms_oUsedList.Clear();
		ms_oFreeList.Clear();
		ms_pEntries.Destroy();
	}

	TModelRegistryEntry* TModelRegistry::CreateModel(const char* a_szFileName, TModelRef& a_rModelRef, TTRB* a_pAssetTRB)
	{
		if (ms_oFreeList.IsEmpty())
		{
			TASSERT(TFALSE, "Couldn't create new model since the registry is all used");
			return TNULL;
		}

		char filepath[248];
		auto iFileNameLen = TStringManager::String8Length(a_szFileName);
		TStringManager::String8Copy(filepath, a_szFileName);

		for (TUINT i = 0; i < iFileNameLen; i++)
		{
			if (filepath[i] == '/')
			{
				filepath[i] = '\\';
			}
		}

		auto crc32 = TUtil::CRC32(TREINTERPRETCAST(unsigned char*, filepath), iFileNameLen);

		for (auto it = ms_oUsedList.Begin(); it != ms_oUsedList.End(); it++)
		{
			if (it->GetCRC32() == crc32)
			{
				a_rModelRef.m_pModel = it->GetModel();
				it->IncRefCount();

				return it;
			}
		}

		auto pEntry = ms_oFreeList.PopBack();
		ms_oUsedList.PushFront(pEntry);

		const char* szSkeletonSymbolName;
		TUINT32 uiSymbolNameOffset = 0;

		if (!a_pAssetTRB || !TModel::GetSkeletonAssetSymbolName(filepath, szSkeletonSymbolName, uiSymbolNameOffset, a_pAssetTRB))
		{
			a_pAssetTRB = TNULL;
			szSkeletonSymbolName = filepath;
			uiSymbolNameOffset = -1;
		}

		auto pModel = TRenderInterface::GetSingleton()->CreateModel3(szSkeletonSymbolName, TTRUE, a_pAssetTRB, uiSymbolNameOffset);
		pEntry->Create(crc32, pModel);

		return pEntry;
	}

	TBOOL TModelRef::Create(const char* a_szFileName, TTRB* a_pTRB)
	{
		m_pModel = TNULL;
		m_pEntry = TModelRegistry::CreateModel(a_szFileName, *this, a_pTRB);

		return m_pModel != TNULL;
	}

}
