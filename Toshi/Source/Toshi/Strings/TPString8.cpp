#include "ToshiPCH.h"
#include "TPString8.h"

namespace Toshi {

	void TPString8Pool::Get(TPooledString8*& a_pOutString, const char* a_szString, bool* a_pWasInPool)
	{
		a_pOutString = TNULL;

		if (a_szString != TNULL)
		{
			auto iStrLen = TStringManager::String8Length(a_szString);

			if (iStrLen != 0)
			{
				TPooledString8* pResult = *m_oMap.Find(a_szString);

				if (pResult != m_oMap.End()->GetSecond())
				{
					if (a_pWasInPool)
						*a_pWasInPool = TTRUE;

					a_pOutString = pResult;
					pResult->IncRefCount();
				}
				else
				{
					TPooledString8* pString = m_pAllocator->New<TPooledString8>(a_szString, this, m_pAllocator);
					m_oMap.Insert(pString->GetString8().GetString(), pString);

					a_pOutString = pString;
					pString->IncRefCount();
				}
			}
		}
	}

	TPString8Pool::TPString8Pool(int a_iUnknown1, int a_iUnknown2, T2Allocator* a_pAllocator, void* m_pUnknown3) :
		m_pAllocator(a_pAllocator),
		m_oMap(a_pAllocator)
	{
		TTODO("Initialise some unknown values");

		for (auto it = TPString8Initialiser::Head(); it != TNULL; it = it->Next())
		{
			it->Initialise(this);
		}
	}

	TPString8Initialiser::TPString8Initialiser(StringMap* a_pStrings, size_t a_iStringCount, TBOOL a_bFlag)
	{
		m_iCount = a_iStringCount;
		m_pStrings = a_pStrings;
		m_pNextInitialiser = ms_pHeadInitialiser;
		m_bFlag = a_bFlag;
		ms_pHeadInitialiser = this;
	}

	void TPString8Initialiser::Initialise(TPString8Pool* a_pStringPool)
	{
		if (m_bFlag)
		{
			for (size_t i = 0; i < m_iCount; i++)
			{
				if (m_pStrings[i].m_pString8)
				{
					m_pStrings[i].m_pString8->SetPooledString(TNULL);
				}
			}
		}

		for (size_t i = 0; i < m_iCount; i++)
		{
			TPString8 string(a_pStringPool, m_pStrings[i].m_szCString);

			if (string.GetPooledString() &&
				string.GetPooledString() != m_pStrings[i].m_pString8->GetPooledString())
			{
				*m_pStrings[i].m_pString8 = string;
			}
		}
	}

}
