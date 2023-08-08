#pragma once

#include "Toshi/Core/TFreeList.h"
#include <Toshi/Strings/TCStringPool.h>

namespace Toshi
{
#ifdef TOSHI_ENABLE_DEPRECATED
	class TPooledCString
	{
	public:
		TPooledCString()
		{
			m_iCount = 0;
			m_pCStringPool = TNULL;
		}

		TPooledCString(const char* a_szString, TCStringPool* a_pStringPool) : m_oString(a_szString)
		{
			m_iCount = 0;
			m_pCStringPool = a_pStringPool;
		}

		~TPooledCString()
		{
			if (m_pCStringPool)
			{
				m_pCStringPool->Remove(*this);
			}
			delete m_oString;
		}

		static TFreeList& GetFreeList()
		{
			return ms_oFreeList;
		}

		static void* operator new(uint32_t size, char*, int)
		{
			return ms_oFreeList.New(size);
		}

		static void* operator new(uint32_t size)
		{
			return ms_oFreeList.New(size);
		}

		static void* operator new(uint32_t size, void* data)
		{
			// No fuck you
			return data;
		}

		static void operator delete(void* data)
		{
			ms_oFreeList.Delete(data);
		}

	public:

		void Delete()
		{
			if (m_pCStringPool)
			{
				m_pCStringPool->Remove(*this);
			}
			delete m_oString;
			ms_oFreeList.Delete(this);
		}

	public:

		int m_iCount;                 // 0x0
		TString8 m_oString;           // 0x4
		TCStringPool* m_pCStringPool;  // 0xC

		static TFreeList ms_oFreeList;
	};
#endif TOSHI_ENABLE_DEPRECATED
}
