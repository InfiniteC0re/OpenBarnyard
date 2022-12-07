#include "ToshiPCH.h"
#include "T2ResourceManager.h"

const int MAX_RESOURCE_NAME = 48;

namespace Toshi
{
	void T2ResourceManager::IncRefCount(int a_iID)
	{
		if (a_iID == T2ResourcePtr::IDINVALID) return;
		m_pData[a_iID].m_iRefCount++;
	}

	void T2ResourceManager::DecRefCount(int a_iID)
	{
		if (a_iID == T2ResourcePtr::IDINVALID) return;
		m_pData[a_iID].m_iRefCount--;
		if (m_pData[a_iID].m_iRefCount != 0) return;
		if (m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED) == 0) return;
		m_pData[a_iID].m_iFlags &= ~T2ResourceData::FLAG_USED;
		m_iNumUsedResources--;
		m_pData[a_iID].m_pResourceName = TNULL;
	}

	void T2ResourceData::Init(const char* a_pName, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData)
	{
		TASSERT(m_fnCreateDestroyCallbk == TNULL, "");
		TASSERT(m_pResourceName[0] == '\0', "");
		TASSERT(m_pData == TNULL, "");
		TASSERT(m_iFlags == 0, "");
		TASSERT(m_iRefCount == 0, "");
		TASSERT(a_fnCreateDestroyCallbk != TNULL, "");
		TASSERT(T2String8::IsLowerCase(a_pName), "");
		TASSERT(T2String8::Length(a_pName) < MAX_RESOURCE_NAME, "");

		T2String8::CopySafe(m_pResourceName, a_pName, MAX_RESOURCE_NAME);

		m_fnCreateDestroyCallbk = a_fnCreateDestroyCallbk;
		m_unk = a_pData;
		m_iFlags |= FLAG_DESTROYED;
	}

	void T2ResourceData::DeInit()
	{
		m_iFlags &= ~FLAG_DESTROYED;
		Unload();
		m_fnCreateDestroyCallbk = TNULL;
		m_iFlags = 0;
		if (m_iRefCount == 0)
		{
			m_unk = TNULL;
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

	void T2ResourceData::Unload()
	{
		if ((m_iFlags & FLAG_LOADING) != 0)
		{
			m_iFlags &= ~(FLAG_LOADING | FLAG_INITIALISED);
			if (m_trb != TNULL)
			{
				TASSERT(TNULL != m_fnCreateDestroyCallbk, "");

				m_fnCreateDestroyCallbk(m_unk, m_trb, false);

				m_trb->Close();
				m_trb->~TTRB();
				m_trb = TNULL;
			}
			m_pData = TNULL;
		}
	}

	void* T2ResourceData::GetData()
	{
		while ((m_iFlags & FLAG_INITIALISED) == 0)
		{
			Sleep(100);
		}
		return m_pData;
	}

	T2ResourceManager::T2ResourceManager(int a_iMaxNumResources)
	{
		m_iNumUsedResources = 0;
		m_iUnk = 0;
		m_iMaxNumResources = a_iMaxNumResources;
		m_pData = new T2ResourceData[m_iMaxNumResources];
		TUtil::MemClear(m_pData, m_iMaxNumResources * sizeof(T2ResourceData));
		m_pData[0].m_iFlags |= T2ResourceData::FLAG_DESTROYED;
	}

	T2ResourceManager::~T2ResourceManager()
	{
		delete[] m_pData;
	}

	T2ResourceData& T2ResourceManager::GetResourceData(int a_iID)
	{
		TASSERT(a_iID != T2ResourcePtr::IDINVALID, "Invalid ID");
		TASSERT(a_iID >= 0, "ID must be greater than 0");
		TASSERT(a_iID < m_iMaxNumResources, "");
		TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED), "");
		TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED), "");
		return m_pData[a_iID];
	}

	void* T2ResourceManager::GetData(int a_iID)
	{
		TASSERT(a_iID >= 0, "ID must be greater than 0");
		TASSERT(a_iID < m_iMaxNumResources, "");
		TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED), "");
		TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED), "");
		return a_iID == T2ResourcePtr::IDINVALID ? TNULL : m_pData[a_iID].GetData();
	}

	int T2ResourceManager::CreateResource(const char* resourceName, void* unk, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData)
	{
		int iID = FindUnusedResource();
		if (m_iMaxNumResources <= iID)
		{
			TOSHI_CORE_ERROR("couldnt create resource \"%s\" because we have reached our max (%d)", resourceName, m_iMaxNumResources);
		}
		TASSERT(iID >= 0, "");
		TASSERT(iID < m_iMaxNumResources, "");
		TASSERT(TFALSE == m_pData[iID].HasFlag(T2ResourceData::FLAG_INITIALISED), "");
		m_iNumUsedResources++;
		m_pData[iID].Init(resourceName, a_fnCreateDestroyCallbk, a_pData);
		m_pData[iID].SetLoadedData(unk);
		m_pData[iID].m_trb2 = TAssetInit::g_pCurrentTRB;

		return iID;
	}

	void T2ResourceManager::DestroyResource(int a_iID)
	{
		if (a_iID != T2ResourcePtr::IDINVALID)
		{
			m_pData[a_iID].DeInit();
			if (m_pData[a_iID].m_pData == TNULL) m_iNumUsedResources--;
		}
	}

	int T2ResourceManager::FindUnusedResource()
	{
		for (size_t i = m_iUnk; i < m_iMaxNumResources; i++)
		{
			if (m_pData[i].HasFlag(T2ResourceData::FLAG_DESTROYED | T2ResourceData::FLAG_USED) && m_pData[i].GetData() == TNULL)
			{
				m_iUnk = i + 1;
				return i;
			}
		}

		for (size_t i = 0; i < m_iUnk; i++)
		{
			if (m_pData[i].HasFlag(T2ResourceData::FLAG_DESTROYED | T2ResourceData::FLAG_USED) && m_pData[i].GetData() == TNULL)
			{
				m_iUnk = i + 1;
				return i;
			}
		}

		TASSERT(!"Should never get here", "");
		return T2ResourcePtr::IDINVALID;
	}

	void T2Resource::CreateResource(const char* resourceName, void* unk, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData)
	{
		// since CreateResource is a static function it cannot use m_iID (commented because couldn't compile)
		//if (m_iID == T2ResourcePtr::IDINVALID)
		//{
		//	m_iID = T2ResourceManager::GetSingleton()->CreateResource(resourceName, unk, a_fnCreateDestroyCallbk, a_pData);
		//}
	}

	void T2Resource::DestroyResource()
	{
		T2ResourceManager::GetSingleton()->DestroyResource(m_iID);
	}
}