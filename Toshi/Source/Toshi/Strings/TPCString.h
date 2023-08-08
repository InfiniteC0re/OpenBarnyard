#pragma once
#include <Toshi/Strings/TString8.h>
#include "Toshi/Strings/TPooledCString.h"

namespace Toshi
{
#ifdef TOSHI_ENABLE_DEPRECATED
	class TPCString
	{
	public:
		TPCString() 
		{ 
			m_pPooledString = TNULL; 
		}

		TPCString(const TPCString& other) : TPCString()
		{
			if (other.GetStringPool())
			{
				m_pPooledString = other.m_pPooledString;
				m_pPooledString->m_iCount++;
			}
		}

		TPCString(TPooledCString* a_rPooledCString)
		{
			m_pPooledString = a_rPooledCString;
			if (m_pPooledString)
			{
				m_pPooledString->m_iCount++;
			}
		}

		~TPCString()
		{
			if (m_pPooledString)
			{
				m_pPooledString->m_iCount--;
				if (m_pPooledString->m_iCount == 0) m_pPooledString->Delete();
				m_pPooledString = TNULL;
			}
		}

		operator const TString8* () const
		{
			if (m_pPooledString)
			{
				return &GetPtr()->m_oString;
			}
			return &ms_sEmpty;
		}

		bool operator==(const TPCString& other) const
		{
			return GetPtr() == other.GetPtr();
		}

		bool operator!=(const TPCString& other) const
		{
			return GetPtr() != other.GetPtr();
		}

		TPCString& operator=(const TPCString& other)
		{
			if (m_pPooledString != other.m_pPooledString)
			{
				if (GetPtr())
				{
					m_pPooledString->m_iCount--;
					if (m_pPooledString->m_iCount == 0) m_pPooledString->Delete();
				}
				m_pPooledString = other.m_pPooledString;
				if (other.m_pPooledString)
				{
					m_pPooledString->m_iCount++;
				}
			}

			return *this;
		}

		bool operator<(const TPCString& other) const
		{
			return Compare(other) == -1;
		}

		const TString8* operator->() const
		{
			if (m_pPooledString)
			{
				return &GetPtr()->m_oString;
			}
			return &ms_sEmpty;
		}

		const TString8& operator*() const
		{
			if (m_pPooledString)
			{
				return GetPtr()->m_oString;
			}
			return ms_sEmpty;
		}


	public:

		int Compare(const TPCString& other) const;

		inline TPooledCString* GetPtr() const { return m_pPooledString; }
		inline TCStringPool* GetStringPool() const { return m_pPooledString->m_pCStringPool; }
		inline TString8& GetVolatileCString() const { TASSERT(GetPtr() != TNULL); TASSERT(GetPtr()->m_pCStringPool == TNULL); return GetPtr()->m_oString; }
		inline const TString8& GetCString() const { return !m_pPooledString ? ms_sEmpty : m_pPooledString->m_oString; }
		inline TBOOL IsEmpty() const { return !m_pPooledString ? TTRUE : m_pPooledString->m_oString.IsEmpty(); }

		TPooledCString* m_pPooledString; // 0x0

		static inline const TString8 ms_sEmpty = TString8();
	};
#endif TOSHI_ENABLE_DEPRECATED
}


