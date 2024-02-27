#pragma once
#ifndef __TOSHI_TPSTRING8_H__
#define __TOSHI_TPSTRING8_H__
#endif

#include "TString8.h"
#include "TStringManager.h"
#include "Toshi/TRefCounted.h"
#include "Toshi/T2Allocator.h"
#include "Toshi/T2Map.h"
#include "Toshi/TSystem.h"

#define TPSTRING8_DECLARE(STR) extern Toshi::TPString8 g_str_##STR
#define TPS8(STR) (g_str_##STR)

namespace Toshi {

	class TPString8Pool;

	class TPooledString8 : public TRefCounted
	{
	public:
		class Comparator
		{
		public:
			static TBOOL IsEqual(const TCHAR* a, const TCHAR* b)
			{
				return TStringManager::String8Compare(a, b) == 0;
			}

			static TBOOL IsGreater(const TCHAR* a, const TCHAR* b)
			{
				return TStringManager::String8Compare(a, b) > 0;
			}

			static TBOOL IsLess(const TCHAR* a, const TCHAR* b)
			{
				return TStringManager::String8Compare(a, b) < 0;
			}

			static TBOOL IsLessOrEqual(const TCHAR* a, const TCHAR* b)
			{
				return TStringManager::String8Compare(a, b) <= 0;
			}

			static TBOOL IsGreaterOrEqual(const TCHAR* a, const TCHAR* b)
			{
				return TStringManager::String8Compare(a, b) >= 0;
			}
		};

		friend class TPString8;

	public:
		TPooledString8(const TCHAR* a_szString, TPString8Pool* a_pPool, T2Allocator* a_pAllocator) :
			m_oString(a_szString, a_pAllocator)
		{
			m_pPool = a_pPool;
		}

		void Delete();

		const TString8& GetString8() const
		{
			return m_oString;
		}

		TPString8Pool* GetStringPool() const
		{
			return m_pPool;
		}

	private:
		TString8 m_oString;
		TPString8Pool* m_pPool;
	};

	class TPString8Pool
	{
	public:
		// Doesn't initialise strings from TPString8Initialiser
		TPString8Pool(T2Allocator* a_pAllocator);

		// Initialises strings from TPString8Initialiser
		TPString8Pool(TINT a_iUnknown1, TINT a_iUnknown2, T2Allocator* a_pAllocator, void* m_pUnknown3);

		void Get(TPooledString8*& a_pOutString, const TCHAR* a_szString, bool* a_pWasInPool = TNULL);

		void Remove(TPooledString8* a_pString)
		{
			TASSERT(a_pString->GetRefCount() == 0);
			m_oMap.Remove(a_pString->GetString8());
		}

		void InitialiseStatic()
		{
			TIMPLEMENT();
		}

		T2Allocator* GetAllocator() const
		{
			return m_pAllocator;
		}

		T2Map<const TCHAR*, TPooledString8*, TPooledString8::Comparator>::Iterator Begin()
		{
			return m_oMap.Begin();
		}

		T2Map<const TCHAR*, TPooledString8*, TPooledString8::Comparator>::Iterator End()
		{
			return m_oMap.End();
		}

	private:
		T2Allocator* m_pAllocator;
		TUINT m_Unk1;
		T2Map<const TCHAR*, TPooledString8*, TPooledString8::Comparator> m_oMap;
		TUINT m_Unk2;
		TUINT m_Unk3;
		TUINT m_Unk4;
		TUINT m_Unk5;
	};

	class TPString8
	{
	public:
		class Comparator
		{
		public:
			static TBOOL IsEqual(const TPString8& a, const TPString8& b)
			{
				return a.GetPooledString() == b.GetPooledString();
			}

			static TBOOL IsGreater(const TPString8& a, const TPString8& b)
			{
				return a.GetPooledString() > b.GetPooledString();
			}

			static TBOOL IsLess(const TPString8& a, const TPString8& b)
			{
				return a.GetPooledString() < b.GetPooledString();
			}

			static TBOOL IsLessOrEqual(const TPString8& a, const TPString8& b)
			{
				return a.GetPooledString() <= b.GetPooledString();
			}

			static TBOOL IsGreaterOrEqual(const TPString8& a, const TPString8& b)
			{
				return a.GetPooledString() >= b.GetPooledString();
			}
		};

		inline static const TString8 ms_sEmpty = TString8("");

	public:
		TFORCEINLINE TPString8()
		{
			m_pPtr = TNULL;
		}

		TFORCEINLINE TPString8(const TCHAR* a_szString)
		{
			TUtil::GetTPStringPool()->Get(m_pPtr, a_szString, TNULL);
		}

		TFORCEINLINE TPString8(TPString8Pool* a_pPool, const TCHAR* a_szString)
		{
			a_pPool->Get(m_pPtr, a_szString, TNULL);
		}

		TFORCEINLINE TPString8(TPooledString8* a_pPooled)
		{
			m_pPtr = a_pPooled;
			Increment();
		}

		TFORCEINLINE TPString8(const TPString8& a_rOther)
		{
			m_pPtr = a_rOther.m_pPtr;
			Increment();
		}

		TFORCEINLINE TPString8(TPString8&& a_rOther)
		{
			m_pPtr = a_rOther.m_pPtr;
			a_rOther.m_pPtr = TNULL;
		}

		TFORCEINLINE ~TPString8()
		{
			Decrement();
		}

		TFORCEINLINE const TString8& GetString8() const
		{
			return m_pPtr ? m_pPtr->m_oString : ms_sEmpty;
		}

		TFORCEINLINE const TPooledString8* GetPooledString() const
		{
			return m_pPtr;
		}

		TFORCEINLINE TBOOL IsEmpty() const
		{
			return m_pPtr == TNULL;
		}

		TFORCEINLINE void SetPooledString(TPooledString8* a_pPooledString)
		{
			if (m_pPtr != a_pPooledString)
			{
				Decrement();
				m_pPtr = a_pPooledString;
				Increment();
			}
		}

		TFORCEINLINE bool IsEqual(const TPString8& a_Other) const
		{
			return m_pPtr == a_Other.m_pPtr;
		}

		TFORCEINLINE bool IsEqual(const TPString8* a_pOther) const
		{
			return m_pPtr == a_pOther->m_pPtr;
		}

		TFORCEINLINE bool operator==(const TPString8& a_Other) const
		{
			return m_pPtr == a_Other.m_pPtr;
		}

		TFORCEINLINE bool operator!=(const TPString8& a_Other) const
		{
			return m_pPtr != a_Other.m_pPtr;
		}

		TFORCEINLINE TPString8& operator=(TPooledString8* a_pString)
		{
			if (m_pPtr != a_pString)
			{
				Decrement();
				m_pPtr = a_pString;
				Increment();
			}

			return *this;
		}

		TFORCEINLINE TPString8& operator=(const TPString8& other)
		{
			if (m_pPtr != other.m_pPtr)
			{
				Decrement();
				m_pPtr = other.m_pPtr;
				Increment();
			}

			return *this;
		}

		TFORCEINLINE TPString8& operator=(const TPString8* pOther)
		{
			if (m_pPtr != pOther->m_pPtr)
			{
				Decrement();
				m_pPtr = pOther->m_pPtr;
				Increment();
			}

			return *this;
		}

		TFORCEINLINE TPString8& operator=(const TCHAR* a_szString)
		{
			Decrement();
			TUtil::GetTPStringPool()->Get(m_pPtr, a_szString, TNULL);
			return *this;
		}

		TFORCEINLINE operator const TCHAR* () const
		{
			return m_pPtr ? m_pPtr->m_oString.GetString() : ms_sEmpty.GetString();
		}

		TFORCEINLINE operator const TString8*() const
		{
			return m_pPtr ? &m_pPtr->m_oString : &ms_sEmpty;
		}

		TFORCEINLINE operator const TPooledString8*() const
		{
			return m_pPtr;
		}

	private:
		TFORCEINLINE void Increment()
		{
			if (m_pPtr)
			{
				m_pPtr->IncRefCount();
			}
		}

		TFORCEINLINE void Decrement()
		{
			if (m_pPtr && m_pPtr->DecRefCount() == 0)
			{
				m_pPtr->Delete();
			}
		}

	private:
		TPooledString8* m_pPtr;
	};

	class TPString8Initialiser
	{
	public:
		struct StringMap
		{
			TPString8* m_pString8;
			const TCHAR* m_szCString;
		};

	public:
		TPString8Initialiser(StringMap* a_pStrings, TUINT a_iStringCount, TBOOL a_bFlag);

		void Initialise(TPString8Pool* a_pStringPool);

		TPString8Initialiser* Next() const
		{
			return m_pNextInitialiser;
		}

		static TPString8Initialiser* Head()
		{
			return ms_pHeadInitialiser;
		}

	private:
		inline static TPString8Initialiser* ms_pHeadInitialiser;

	private:
		StringMap* m_pStrings;
		TUINT m_iCount;
		TPString8Initialiser* m_pNextInitialiser;
		TBOOL m_bFlag;
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
