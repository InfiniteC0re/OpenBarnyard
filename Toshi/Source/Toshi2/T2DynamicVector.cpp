#include "ToshiPCH.h"
#include "T2DynamicVector.h"

void Toshi::T2GenericDynamicVector::Reallocate(int a_iNewSize, int a_iCount)
{
	if (a_iNewSize != m_iElementSize)
	{
		TASSERT(m_pAllocator != TNULL, "Cannot reallocate a T2DynamicVector with no allocator specified!");

		void* elements = TNULL;

		if (a_iNewSize > 0)
		{
			elements = m_pAllocator->Malloc(a_iNewSize * a_iCount);

			TUtil::MemCopy(elements, m_poElements, m_iNumElements <= a_iNewSize ? m_iNumElements : a_iNewSize);
		}
		m_iElementSize = a_iNewSize;
	}
}
