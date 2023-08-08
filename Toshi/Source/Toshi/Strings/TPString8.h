#pragma once
#include "TString8.h"
#include "TStringManager.h"
#include "Toshi2/T2Allocator.h"
#include "Toshi2/T2Map.h"
#include "Toshi/Core/TSystem.h"

namespace Toshi {

	class TPString8Pool;

	class TPooledString8
	{
	public:
		class Comparator
		{
		public:
			static TBOOL IsEqual(const char* a, const char* b)
			{
				return TStringManager::String8Compare(a, b) == 0;
			}

			static TBOOL IsGreater(const char* a, const char* b)
			{
				return TStringManager::String8Compare(a, b) > 0;
			}

			static TBOOL IsLess(const char* a, const char* b)
			{
				return TStringManager::String8Compare(a, b) < 0;
			}

			static TBOOL IsLessOrEqual(const char* a, const char* b)
			{
				return TStringManager::String8Compare(a, b) <= 0;
			}

			static TBOOL IsGreaterOrEqual(const char* a, const char* b)
			{
				return TStringManager::String8Compare(a, b) >= 0;
			}
		};

		friend class TPString8;

	public:
		TPooledString8(const char* a_szString, TPString8Pool* a_pPool, T2Allocator* a_pAllocator) :
			m_oString(a_szString, a_pAllocator)
		{
			m_iRefCount = 0;
			m_pPool = a_pPool;
		}

		void Delete();

		int IncRefCount()
		{
			return ++m_iRefCount;
		}

		int DecRefCount()
		{
			return --m_iRefCount;
		}

		int GetRefCount() const
		{
			return m_iRefCount;
		}

		const TString8& GetString8() const
		{
			return m_oString;
		}

		TPString8Pool* GetStringPool() const
		{
			return m_pPool;
		}

	private:
		int m_iRefCount;
		TString8 m_oString;
		TPString8Pool* m_pPool;
	};

	class TPString8PoolAllocator : public T2GlobalAllocator
	{
	public:
		TPooledString8* AllocateString(const char* a_szString, TPString8Pool* a_pPool, T2Allocator* a_pAllocator)
		{
			return T2GlobalAllocator::New<TPooledString8>(a_szString, a_pPool, a_pAllocator);
		}
	};

	class TPString8Pool
	{
	public:
		inline static TPString8PoolAllocator s_Allocator;

	public:
		static void Create()
		{
			TSystemManager::SetStringPool(new TPString8Pool(1024, 0, &s_Allocator, 0));
			TTODO("FUN_006c1a60 in Barnyard");
		}

		void Get(TPooledString8*& a_pOutString, const char* a_szString, bool* a_pWasInPool = TNULL);

		void Remove(TPooledString8* a_pString)
		{
			TASSERT(a_pString->GetRefCount() == 0);
			m_oMap.Remove(a_pString->GetString8());
		}

		TPString8PoolAllocator* GetAllocator() const
		{
			return m_pAllocator;
		}

	private:
		TPString8Pool(int a_iUnknown1, int a_iUnknown2, TPString8PoolAllocator* a_pAllocator, void* m_pUnknown3);

	private:
		TPString8PoolAllocator* m_pAllocator;
		T2Map<const char*, TPooledString8*, TPooledString8::Comparator> m_oMap;
	};

	class TPString8
	{
	public:
		__forceinline TPString8()
		{
			m_pPtr = TNULL;
		}

		__forceinline TPString8(const char* a_szString)
		{
			TSystemManager::GetStringPool()->Get(m_pPtr, a_szString, TNULL);
		}

		__forceinline TPString8(TPString8Pool* a_pPool, const char* a_szString)
		{
			a_pPool->Get(m_pPtr, a_szString, TNULL);
		}

		__forceinline TPString8(TPooledString8* a_pPooled)
		{
			m_pPtr = a_pPooled;
			m_pPtr->IncRefCount();
		}

		__forceinline TPString8(const TPString8& other)
		{
			m_pPtr = other.m_pPtr;
			m_pPtr->IncRefCount();
		}

		TPString8(TPString8&& other) = delete;

		__forceinline ~TPString8()
		{
			Decrement();
		}

		__forceinline const TString8& GetString8() const
		{
			return m_pPtr->m_oString;
		}

		__forceinline const TPooledString8* GetPooledString() const
		{
			return m_pPtr;
		}

		__forceinline void SetPooledString(TPooledString8* a_pPooledString)
		{
			Decrement();
			m_pPtr = a_pPooledString;
		}

		__forceinline TPString8& operator=(TPooledString8* a_pString)
		{
			Decrement();
			m_pPtr = a_pString;
			m_pPtr->IncRefCount();

			return *this;
		}

		__forceinline TPString8& operator=(const TPString8& other)
		{
			Decrement();
			m_pPtr = other.m_pPtr;
			m_pPtr->IncRefCount();

			return *this;
		}

		__forceinline TPString8& operator=(const char* a_szString)
		{
			Decrement();
			TSystemManager::GetStringPool()->Get(m_pPtr, a_szString, TNULL);

			return *this;
		}

		__forceinline operator const TString8*() const
		{
			return &m_pPtr->m_oString;
		}

		__forceinline operator const TPooledString8*() const
		{
			return m_pPtr;
		}

	private:
		__forceinline void Decrement()
		{
			if (m_pPtr && m_pPtr->DecRefCount() == 0)
			{
				m_pPtr->Delete();
			}
		}

	private:
		TPooledString8* m_pPtr;
	};

	inline void TPooledString8::Delete()
	{
		if (m_pPool)
		{
			m_pPool->Remove(this);
			m_pPool->GetAllocator()->Delete(this);
		}
		else
		{
			this->~TPooledString8();
			TIMPLEMENT_D("FUN_006c1880 in Barnyard");
		}
	}

}
