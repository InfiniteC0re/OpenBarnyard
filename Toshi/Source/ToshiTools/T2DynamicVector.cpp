#include "ToshiPCH.h"
#include "T2DynamicVector.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void T2GenericDynamicVector::Reallocate( TINT a_iNewSize, TINT a_iElementSize )
{
	if ( a_iNewSize != m_iAllocSize )
	{
		TASSERT( m_pAllocator != TNULL, "Cannot reallocate a T2DynamicVector with no allocator specified!" );

		void* elements = TNULL;

		if ( 0 < a_iNewSize )
		{
			elements = m_pAllocator->Malloc( a_iNewSize * a_iElementSize );
			TUtil::MemCopy( elements, m_poElements, m_iNumElements <= a_iNewSize ? m_iNumElements * a_iElementSize : a_iNewSize * a_iElementSize );
		}

		m_iAllocSize = a_iNewSize;
		m_iNumElements = TMath::Min( m_iNumElements, a_iNewSize );

		if ( m_poElements )
		{
			m_pAllocator->Free( m_poElements );
		}

		m_poElements = elements;
	}
}

void T2GenericDynamicVector::Grow( TINT a_iNumElements, TINT a_iElementSize )
{
	TINT newSize = m_iNumElements + a_iNumElements;
	TINT curSize = m_iAllocSize;

	if ( curSize < newSize )
	{
		TASSERT( m_iGrowSize != 0 );

		while ( curSize < newSize )
		{
			if ( m_iGrowSize == -1 )
			{
				curSize = m_iAllocSize * 2 < 3 ? 2 : m_iAllocSize * 2;
			}
			else
			{
				curSize += m_iGrowSize;
			}
		}

		Reallocate( curSize, a_iElementSize );
	}
}

TOSHI_NAMESPACE_END
