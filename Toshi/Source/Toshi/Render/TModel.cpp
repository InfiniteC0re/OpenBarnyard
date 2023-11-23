#include "ToshiPCH.h"
#include "TModel.h"
#include "TModelManager.h"
#include TOSHI_MULTIRENDER(TRenderInterface)

namespace Toshi {
	
	TModel::TModel()
	{
		m_Flags = Flags::None;
		m_Unk1 = 0;
		m_iLODCount = 0;
		m_fUnknown = -1.0f;
		m_pSkeleton = TNULL;
		m_pCollisionData = TNULL;
		m_pTRB = TNULL;
		m_bFreeOnUnload = TTRUE;
		m_pName = "unknown";
		TUtil::MemClear(m_LODLevels, sizeof(m_LODLevels));
        m_pDataHeader = TNULL;
		m_fUnk2 = 5.0f;
		m_fUnk3 = 20.0f;
		m_fUnk4 = 40.0f;
		m_fUnk5 = 80.0f;
	}

	TModel::~TModel()
	{
		TASSERT(TNULL == m_pTRB);
		auto pResourceManager = T2ResourceManager::GetSingleton();

		if (GetResourceId() != T2ResourcePtr::IDINVALID)
		{
			auto pModelManager = TModelManager::GetSingleton();
			pModelManager->RemoveModel(GetResourceId());
		}

		DestroyResource();
	}

	void TModel::Create(const char* name, TBOOL bLoadImmediately)
	{
		TASSERT(TFALSE == IsCreated());
		m_Flags.Set(Flags::Created);

		TASSERT(TFALSE == bLoadImmediately);

		CreateResource(name);
		m_pName = name;
	}

	void TModel::Delete()
	{
		Unload();
		delete this;
	}

	void TModel::Unload()
	{
		TIMPLEMENT();
	}

	TBOOL TModel::LoadTRB()
	{
		TTMDBase::SkeletonHeader* pSkeletonHeader = m_pTRB->CastSymbol<TTMDBase::SkeletonHeader>(TranslateSymbolName("SkeletonHeader"));
		TSkeleton* pSkeleton                      = m_pTRB->CastSymbol<TSkeleton>(TranslateSymbolName("Skeleton"));
		TModelCollision* pModelCollision          = m_pTRB->CastSymbol<TModelCollision>(TranslateSymbolName("Collision"));

		CreateSkeleton(pSkeletonHeader, pSkeleton, TTRUE);
		CreateCollision(pModelCollision);

		TASSERT(TNULL != sm_pTRBLoadCallback);

		TBOOL bLoadRes = sm_pTRBLoadCallback(*m_pTRB, this);
		TASSERT(bLoadRes, "Failed To Load Model TRB");

		m_Flags.Set(Flags::TrbLoaded);
		return TTRUE;
	}

	TBOOL TModel::LoadTRB(TTRB* pTRB, TBOOL bFreeOnUnload)
	{
		SetTRB(pTRB, bFreeOnUnload);
		return LoadTRB();
	}

	TBOOL TModel::LoadTRBFile(const char* filepath)
	{
		if (m_bFreeOnUnload && m_pTRB != TNULL)
		{
			m_pTRB->Close();
			delete m_pTRB;
		}

		m_pTRB = new TTRB();
		TTRB::ERROR iError = m_pTRB->Load(filepath);

		if (iError == TTRB::ERROR_OK)
		{
			return LoadTRB();
		}

		return TFALSE;
	}

	void TModel::UnloadTRB(TBOOL bFreeTrb)
	{
		if (m_pTRB != TNULL)
		{
			if (bFreeTrb && m_bFreeOnUnload)
			{
				m_pTRB->Close();
				delete m_pTRB;
			}

			m_pTRB = TNULL;
		}

		m_Flags.Unset(Flags::TrbLoaded);
		m_pCollisionData = TNULL;
		m_pSkeleton = TNULL;
	}

	void TModel::CreateResource(const char* name)
	{
		T2Resource::CreateResource(name, this, ResourceCallback, this);
		
		auto pModelManager = TModelManager::GetSingleton();
		pModelManager->AddModel(GetResourceId());
	}

	void TModel::CreateSkeleton(TTMDBase::SkeletonHeader* pSkeletonHeader, TSkeleton* pSkeleton, TBOOL bLoadAnimations)
	{
		if (pSkeletonHeader != TNULL)
		{
			TASSERT(TNULL == m_pSkeleton);
			m_pSkeleton = pSkeleton;

			if (bLoadAnimations && pSkeletonHeader->m_pTKLName != TNULL)
			{
				/*auto& keyframeLibMgr = TRender::GetSingletonSafe()->GetKeyframeLibraryManager();
				auto pKeyframeLibrary = keyframeLibMgr.GetLibrary(pSkeletonHeader->m_pTKLName);

				pSkeleton->GetKeyLibraryInstance().CreateEx(
					pKeyframeLibrary,
					pSkeletonHeader->m_iTKeyCount,
					pSkeletonHeader->m_iQKeyCount,
					pSkeletonHeader->m_iSKeyCount,
					pSkeletonHeader->m_iTBaseIndex,
					pSkeletonHeader->m_iQBaseIndex,
					pSkeletonHeader->m_iSBaseIndex
				);*/
			}
		}
	}

	void TModel::CreateCollision(TModelCollision* pModelCollision)
	{
		m_pCollisionData = pModelCollision;
	}

	void* TModel::ResourceCallback(void* pData, TTRB* pTRB, TBOOL bCreated)
	{
		TModel* pModel = TSTATICCAST(TModel*, pData);

		if (bCreated)
		{
			/*auto pRender = TRenderDX11::Interface();
			pRender->WaitForEndOfRender();*/

			pModel->m_pTRB = pTRB;
			pModel->m_Flags.Unset(Flags::TrbLoaded);
			pModel->m_pCollisionData = TNULL;
			pModel->m_pSkeleton = TNULL;
			pModel->m_bFreeOnUnload = TTRUE;
			pModel->LoadTRB();

			return pModel;
		}

		return TNULL;
	}

	TBOOL TModel::GetSkeletonAssetSymbolName(const char* a_szFileName, const char*& a_rSymbolName, TUINT32& a_rStartPos, TTRB* a_pTRB)
	{
		auto iFilePathLength = TStringManager::String8Length(a_szFileName);
		auto iFileNamePos = iFilePathLength - 1;
		a_rStartPos = iFilePathLength;

		while (a_szFileName[iFileNamePos] != '\\' && a_szFileName[iFileNamePos] != '/')
			iFileNamePos--;

		iFileNamePos++;
		char symbolName[64];

		auto iFileNameLength = iFilePathLength - iFileNamePos - 4;
		a_rStartPos = iFileNameLength;

		TStringManager::String8Copy(symbolName, a_szFileName + iFileNamePos, iFileNameLength);
		symbolName[iFileNameLength] = '_';
		symbolName[iFileNameLength + 1] = '\0';
		a_rStartPos = iFileNameLength + 1;

		TStringManager::String8ToLowerCase(symbolName);
		TStringManager::String8Copy(symbolName + a_rStartPos, "Skeleton");

		auto pSymbol = a_pTRB->GetSymbol(symbolName);

		if (pSymbol)
		{
			a_rSymbolName = a_pTRB->GetSymbolName(pSymbol);
			return TTRUE;
		}
		else
		{
			return TFALSE;
		}
	}

	const char* TModel::TranslateSymbolName(const char* symbolName)
	{
		static char s_TranslatedSymbol[512];

		if (sm_SymbolNamePrefix != TNULL)
		{
			T2String8::Format(s_TranslatedSymbol, "%s_%s", sm_SymbolNamePrefix, symbolName);
			return s_TranslatedSymbol;
		}
		
		return symbolName;
	}

}
