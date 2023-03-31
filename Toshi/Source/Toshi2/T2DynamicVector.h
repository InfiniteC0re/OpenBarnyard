#pragma once

#include "T2Allocator.h"

namespace Toshi
{
	class T2GenericDynamicVector
	{
		T2GenericDynamicVector(T2Allocator* a_pAllocator, int a_iElementSize, int a_iGrowSize, int a_iCount)
		{
			m_pAllocator = a_pAllocator;
			m_iGrowSize = a_iGrowSize;
			m_iNumElements = 0;
			m_iElementSize = a_iElementSize;
			m_poElements = a_iElementSize > 0 ? a_pAllocator->Malloc(a_iElementSize * a_iCount) : TNULL;
		}

		void Reallocate(int a_iNewSize, int a_iCount);

		T2Allocator* m_pAllocator; // 0x0
		int m_iGrowSize;           // 0x4
		int m_iNumElements;        // 0x8
		int m_iElementSize;        // 0xC
		void* m_poElements;        // 0x10
	};
}


