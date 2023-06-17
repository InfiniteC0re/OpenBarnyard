#include "ToshiPCH.h"
#include "TModel.h"
#include "TModelManager.h"
#include TOSHI_MULTIRENDER(TRender)

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
		m_pResourceName = "unknown";
		TUtil::MemClear(m_LODLevels, sizeof(m_LODLevels));
		m_Unk6 = 0;
		m_fUnk2 = 5.0f;
		m_fUnk3 = 20.0f;
		m_fUnk4 = 40.0f;
		m_fUnk5 = 80.0f;
	}

	TModel::~TModel()
	{
		TASSERT(TNULL == m_pTRB);
		auto pResourceManager = T2ResourceManager::GetSingletonWeak();

		if (GetResourceId() != T2ResourcePtr::IDINVALID)
		{
			auto pModelManager = TModelManager::GetSingletonWeak();
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
		m_pResourceName = name;
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

	TBOOL TModel::LoadTRBFile(TFile* pFile)
	{
		if (m_bFreeOnUnload && m_pTRB != TNULL)
		{
			m_pTRB->Close();
			delete m_pTRB;
		}

		m_pTRB = new TTRB();
		TTRB::ERROR iError = m_pTRB->Load(pFile);

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
		
		auto pModelManager = TModelManager::GetSingletonWeak();
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
				auto keyframeLibraryManager = TRender::GetSingleton()->GetKeyframeLibraryManager();
				auto pKeyframeLibrary = keyframeLibraryManager.GetLibrary(pSkeletonHeader->m_pTKLName);

				TTODO("Toshi::TKeyframeLibraryInstance::CreateEx");
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
			auto pRender = TRenderDX11::Interface();
			pRender->WaitForEndOfRender();

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

/*
TBOOL Toshi::TModel::LoadTMD(const char* a_tmdPath)
{
	TFile* file = TFile::Create(a_tmdPath);

	if (file != TNULL)
	{
		Header header;
		file->Read(&header, sizeof(Header));

		if (header.m_magic == TMAKEFOUR("TMDL"))
		{

		}

		file->Destroy();
	}

	return TFALSE;
}

void Toshi::TModel::CreateResource(const char* name)
{
	T2Resource::CreateResource(name, this, TNULL, this);
	
}

TBOOL Toshi::TModel::LoadTRBTMD(const char*)
{
	const char* symbol = TranslateSymbolName("SkeletonHeader");
	m_pTRB->GetSymbolAddress(symbol);
	symbol = TranslateSymbolName("Skeleton");
	m_pTRB->GetSymbolAddress(symbol);
	symbol = TranslateSymbolName("Collision");
	m_pTRB->GetSymbolAddress(symbol);
	return TFALSE;
}

TBOOL Toshi::TModel::LoadTrb()
{
	return TFALSE;
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
*/
