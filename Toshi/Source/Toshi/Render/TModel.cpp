#include "ToshiPCH.h"
#include "TModel.h"
#include TOSHI_MULTIRENDER(TRender)

namespace Toshi {
	
	TModel::TModel()
	{
		m_Flags = Flags::None;
		m_iLODCount = 0;
		m_pSkeleton = TNULL;
		m_pCollision = TNULL;
		m_pTRB = TNULL;
		m_bFreeOnUnload = TTRUE;
		m_pResourceName = "unknown";
		TIMPLEMENT();
	}

	TModel::~TModel()
	{
		TASSERT(TNULL == m_pTRB);
		auto pResourceManager = T2ResourceManager::GetSingletonWeak();

		if (GetResourcePtr())
		{
			TTODO("TModelManager::RemoveModel");
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
		m_pCollision = TNULL;
		m_pSkeleton = TNULL;
	}

	void TModel::CreateResource(const char* name)
	{
		T2Resource::CreateResource(name, this, ResourceCallback, this);
		
		auto resourcePtr = GetResourcePtr();
		TTODO("TModelManager::AddModel");
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
		m_pCollision = pModelCollision;
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
			pModel->m_pCollision = TNULL;
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
bool Toshi::TModel::LoadTMD(const char* a_tmdPath)
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
*/
