#include "ToshiPCH.h"
#include "TPString8.h"
#include "TPString8Static.h"

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
					TPooledString8* pString = m_pAllocator->AllocateString(a_szString, this, m_pAllocator);
					m_oMap.Insert(a_szString, pString);

					a_pOutString = pString;
					pString->IncRefCount();
				}
			}
		}
	}

	TPString8Pool::TPString8Pool(int a_iUnknown1, int a_iUnknown2, Allocator* a_pAllocator, void* m_pUnknown3) :
		m_pAllocator(a_pAllocator),
		m_oMap(a_pAllocator)
	{
		TTODO("Initialise some unknown values");

		TPString8StaticPool::Undump(this);
	}

}
