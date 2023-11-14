#include "ToshiPCH.h"
#include "T2DynamicVector.h"

namespace Toshi {

	void T2GenericDynamicVector::Reallocate(int a_iNewSize, int a_iCount)
	{
		if (a_iNewSize != m_iMaxSize)
		{
			TASSERT(m_pAllocator != TNULL, "Cannot reallocate a T2DynamicVector with no allocator specified!");

			void* elements = TNULL;

			if (0 < a_iNewSize)
			{
				elements = m_pAllocator->Malloc(a_iNewSize * a_iCount);
				TUtil::MemCopy(elements, m_poElements, m_iNumElements <= a_iNewSize ? m_iNumElements * a_iCount : a_iNewSize * a_iCount);
			}

			m_iMaxSize = a_iNewSize;
			m_iNumElements = TMath::Min(m_iNumElements, a_iNewSize);

			if (m_poElements)
			{
				m_pAllocator->Free(m_poElements);
			}

			m_poElements = elements;
		}
	}

	void T2GenericDynamicVector::Grow(int a_iNumElements, int a_iCount)
	{
		int newSize = m_iNumElements + a_iNumElements;
		int curSize = m_iMaxSize;

		if (curSize < newSize)
		{
			TASSERT(m_iGrowSize != 0);

			while (curSize < newSize)
			{
				if (m_iGrowSize == -1)
				{
					curSize = m_iMaxSize * 2 < 3 ? 2 : m_iMaxSize * 2;
				}
				else
				{
					curSize += m_iGrowSize;
				}
			}

			Reallocate(curSize, a_iCount);
		}
	}


}
