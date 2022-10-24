#include "ToshiPCH.h"
#include "T2ResourceManager.h"


const int MAX_RESOURCE_NAME = 48;

void Toshi::T2ResourceData::Init(const char* a_pName, t_CreateDestroyCallbk a_fnCreateDestroyCallbk, void* a_pData)
{
	TASSERT(m_fnCreateDestroyCallbk == TNULL, "");
	TASSERT(m_pResourceName[0] == '\0', "");
	TASSERT(m_pData == TNULL, "");
	TASSERT(m_iFlags == 0, "");
	TASSERT(m_iRefCount == 0, "");
	TASSERT(a_fnCreateDestroyCallbk != TNULL, "");
	TASSERT(Toshi2::T2String8::IsLowerCase(a_pName), "");
	TASSERT(Toshi2::T2String8::Length(a_pName) < MAX_RESOURCE_NAME, "");

	Toshi2::T2String8::CopySafe(m_pResourceName, a_pName, MAX_RESOURCE_NAME);

	m_fnCreateDestroyCallbk = a_fnCreateDestroyCallbk;
	m_unk = a_pData;
	m_iFlags |= FLAG_UNK;
}

void Toshi::T2ResourceData::SetLoadedData(void* a_pData)
{
	Unload();
	m_pData = a_pData;
	m_iFlags |= FLAG_LOADING;
	m_iFlags &= ~FLAG_INITIALISED;
}

void Toshi::T2ResourceData::Unload()
{
	if ((m_iFlags & FLAG_LOADING) != 0)
	{
		m_iFlags &= ~FLAG_DESTROYED;
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

void* Toshi::T2ResourceData::GetData()
{
	while ((m_iFlags & FLAG_INITIALISED) == 0)
	{
		Sleep(100);
	}
	return m_pData;
}

Toshi::T2ResourceData& Toshi::T2ResourceManager::GetResourceData(int a_iID)
{
	TASSERT(a_iID != T2ResourcePtr::IDINVALID, "Invalid ID");
	TASSERT(a_iID >= 0, "ID must be greater than 0");
	TASSERT(a_iID < m_iMaxNumResources, "");
	TASSERT(TFALSE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_DESTROYED), "");
	TASSERT(TTRUE == m_pData[a_iID].HasFlag(T2ResourceData::FLAG_INITIALISED), "");
	return m_pData[a_iID];
}
