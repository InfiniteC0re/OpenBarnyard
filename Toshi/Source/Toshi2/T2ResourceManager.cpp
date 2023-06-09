#include "ToshiPCH.h"
#include "T2ResourceManager.h"
#include "Toshi/Render/TAssetInit.h"
#include "Toshi/Core/TError.h"

namespace Toshi
{
	void T2Resource::CreateResource(const char* resourceName, void* pData, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData)
	{
		if (m_iID == T2ResourcePtr::IDINVALID)
		{
			m_iID = T2ResourceManager::GetSingleton()->CreateResource(resourceName, pData, a_fnCreateDestroyCallbk, pCallbkData);
			return;
		}

		TASSERT(TFALSE, "resource already created!");
	}

	void T2Resource::DestroyResource()
	{
		TASSERT(m_iID != T2ResourcePtr::IDINVALID);

		T2ResourceManager::GetSingleton()->DestroyResource(m_iID);
	}

	void T2ResourceData::DeInit()
	{
		TASSERT(HasFlag(FLAG_INITIALISED));
		m_iFlags &= ~FLAG_DESTROYED;
		
		Unload();
		TASSERT(m_pData == TNULL);

		m_fnCreateDestroyCallbk = TNULL;
		m_iFlags = 0;

		if (m_iRefCount == 0)
		{
			m_pResourceName[0] = '\0';
		}
		else
		{
			m_iFlags |= FLAG_USED;
		}
	}

	void T2ResourceData::SetLoadedData(void* a_pData)
	{
		Unload();
		m_pData = a_pData;
		m_iFlags |= FLAG_LOADING;
		m_iFlags &= ~FLAG_INITIALISED;
	}

	void T2ResourceData::Load(const char* filepath)
	{
		if (m_iFlags & FLAG_INITIALISED || m_iFlags & FLAG_LOADING)
			return;

		m_pTRB2 = new TTRB;
		TTRB::ERROR iError = m_pTRB2->Load(filepath);

		if (iError == TTRB::ERROR_OK)
		{
			TASSERT(TNULL != m_fnCreateDestroyCallbk);
			m_pData = m_fnCreateDestroyCallbk(m_pCreateDestroyCallbkData, m_pTRB2, TTRUE);
			
			if (m_pData != TNULL)
			{
				m_iFlags |= FLAG_INITIALISED;
				m_iFlags &= ~FLAG_LOADING;
				return;
			}
		}
		
		m_pTRB2->Close();
		delete m_pTRB2;
		m_pTRB2 = TNULL;
	}

	void T2ResourceData::Unload()
	{
		if (HASFLAG(m_iFlags & FLAG_LOADING))
		{
			m_iFlags &= ~(FLAG_LOADING | FLAG_INITIALISED);

			if (m_pTRB2 != TNULL)
			{
				TASSERT(TNULL != m_fnCreateDestroyCallbk);

				m_fnCreateDestroyCallbk(m_pCreateDestroyCallbkData, m_pTRB2, TFALSE);

				m_pTRB2->Close();
				delete m_pTRB2;
				m_pTRB2 = TNULL;
			}

			m_pData = TNULL;
		}
	}

	void* T2ResourceData::GetData()
	{
		if (!HasFlag(FLAG_INITIALISED))
		{
			TASSERT(HasFlag(FLAG_LOADING));

			while (!HasFlag(FLAG_INITIALISED))
				Sleep(100);
		}

		return m_pData;
	}

	void T2ResourceManager::IncRefCount(int a_iID)
	{
		if (a_iID != T2ResourcePtr::IDINVALID)
		{
			TASSERT(a_iID >= 0);
			TASSERT(a_iID < m_iMaxNumResources);

			m_pData[a_iID].m_iRefCount++;
			TASSERT(m_pData[a_iID].m_iRefCount > 0);
		}
	}

	void T2ResourceManager::DecRefCount(int a_iID)
	{
		if (a_iID != T2ResourcePtr::IDINVALID)
		{
			TASSERT(a_iID >= 0);
			TASSERT(a_iID < m_iMaxNumResources);

			auto pData = &m_pData[a_iID];
			TASSERT(pData->m_iRefCount >= 0);

			if (--pData->m_iRefCount == 0 && pData->HasFlag(T2ResourceData::FLAG_USED))
			{
				pData->m_iFlags = ~T2ResourceData::FLAG_USED;
				pData->m_pResourceName[0] = '\0';
				m_iNumUsedResources--;
			}
		}
	}

	void T2ResourceData::Init(const char* a_pName, T2ResourceManager::t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pCallbkData)
	{
		TASSERT(m_fnCreateDestroyCallbk == TNULL);
		TASSERT(m_pResourceName[0] == '\0');
		TASSERT(m_pData == TNULL);
		TASSERT(m_iFlags == 0);
		TASSERT(m_iRefCount == 0);
		TASSERT(a_fnCreateDestroyCallbk != TNULL);
		TASSERT(T2String8::IsLowerCase(a_pName));
		TASSERT(T2String8::Length(a_pName) < MAX_RESOURCE_NAME);

		T2String8::CopySafe(m_pResourceName, a_pName, MAX_RESOURCE_NAME);

		m_fnCreateDestroyCallbk = a_fnCreateDestroyCallbk;
		m_pCreateDestroyCallbkData = a_pCallbkData;
		m_iFlags |= FLAG_DESTROYED;
	}

	T2ResourceManager::T2ResourceManager(int a_iMaxNumResources)
	{
		m_iMaxNumResources = a_iMaxNumResources;
		m_iNumUsedResources = 0;
		m_iUnk = 0;

		m_pData = new T2ResourceData[m_iMaxNumResources];
		TUtil::MemClear(m_pData, m_iMaxNumResources * sizeof(T2ResourceData));

		m_pData[0].m_iFlags |= T2ResourceData::FLAG_DESTROYED;
	}

	T2ResourceManager::~T2ResourceManager()
	{
		delete[] m_pData;
	}

	T2ResourceData* T2ResourceManager::GetResourceData(int a_iID)
	{
		TASSERT(a_iID != T2ResourcePtr::IDINVALID, "Invalid ID");
		TASSERT(a_iID >= 0, "ID must be greater than 0");
		TASSERT(a_iID < m_iMaxNumResources);
		TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED));
		TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED));

		return &m_pData[a_iID];
	}

	void* T2ResourceManager::GetData(int a_iID)
	{
		TASSERT(a_iID >= 0, "ID must be greater than 0");
		TASSERT(a_iID < m_iMaxNumResources);
		TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED));
		TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED));

		return a_iID == T2ResourcePtr::IDINVALID ? TNULL : m_pData[a_iID].GetData();
	}

	int T2ResourceManager::CreateResource(const char* resourceName, void* pData, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* pCallbkData)
	{
		int iID = FindUnusedResource();
		
		if (m_iMaxNumResources <= iID)
		{
			TError::GetSingletonWeak()->AddError("couldnt create resource \"%s\" because we have reached our max (%d)", resourceName, m_iMaxNumResources);
			return 0;
		}

		TASSERT(iID >= 0);
		TASSERT(iID < m_iMaxNumResources);
		TASSERT(TFALSE == m_pData[iID].HasFlag(T2ResourceData::FLAG_INITIALISED));

		m_iNumUsedResources++;
		m_pData[iID].Init(resourceName, a_fnCreateDestroyCallbk, pCallbkData);
		m_pData[iID].SetLoadedData(pData);
		m_pData[iID].m_pTRB1 = TAssetInit::g_pCurrentTRB;

		return iID;
	}

	void T2ResourceManager::DestroyResource(int a_iID)
	{
		if (a_iID != T2ResourcePtr::IDINVALID)
		{
			TASSERT(a_iID >= 0);
			TASSERT(a_iID < m_iMaxNumResources);

			m_pData[a_iID].DeInit();
			
			if (m_pData[a_iID].m_iRefCount == 0)
			{
				m_iNumUsedResources--;
			}
		}
	}

	int T2ResourceManager::FindUnusedResource()
	{
		TASSERT(m_iNumUsedResources < m_iMaxNumResources);

		for (int i = m_iUnk; i < m_iMaxNumResources; i++)
		{
			if (m_pData[i].HasFlag(T2ResourceData::FLAG_DESTROYED | T2ResourceData::FLAG_USED) && m_pData[i].GetData() == TNULL)
			{
				m_iUnk = i + 1;
				return i;
			}
		}

		for (int i = 0; i < m_iUnk; i++)
		{
			if (m_pData[i].HasFlag(T2ResourceData::FLAG_DESTROYED | T2ResourceData::FLAG_USED) && m_pData[i].GetData() == TNULL)
			{
				m_iUnk = i + 1;
				return i;
			}
		}

		TASSERT(TFALSE, "Should never get here");
		return T2ResourcePtr::IDINVALID;
	}
}