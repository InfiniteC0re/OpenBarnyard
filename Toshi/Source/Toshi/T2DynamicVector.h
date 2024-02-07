#pragma once

#include "T2Allocator.h"

namespace Toshi
{
	class T2GenericDynamicVector
	{
	protected:
		T2GenericDynamicVector(T2Allocator* a_pAllocator, TINT a_iMaxSize, TINT a_iGrowSize, TINT a_iElementSize)
		{
			m_pAllocator = a_pAllocator;
			m_iGrowSize = a_iGrowSize;
			m_iNumElements = 0;
			m_iMaxSize = a_iMaxSize;
			m_poElements = a_iMaxSize > 0 ? a_pAllocator->Malloc(a_iMaxSize * a_iElementSize) : TNULL;
		}

		T2GenericDynamicVector(void* a_poElements, TINT a_iMaxSize, TINT a_iNumElements)
		{
			m_pAllocator = TNULL;
			m_iGrowSize = 0;
			m_iNumElements = a_iNumElements;
			m_iMaxSize = a_iMaxSize;
			m_poElements = a_poElements;
		}

		void Reallocate(TINT a_iNewSize, TINT a_iElementSize);
		void Grow(TINT a_iNumElements, TINT a_iElementSize);

	protected:
		T2Allocator* m_pAllocator; // 0x0
		TINT m_iGrowSize;          // 0x4
		TINT m_iNumElements;       // 0x8
		TINT m_iMaxSize;           // 0xC
		void* m_poElements;        // 0x10
	};

	template<class T>
	class T2DynamicVector : public T2GenericDynamicVector
	{
	public:
		class Iterator
		{
		public:
			friend T2DynamicVector;

		public:
			Iterator()
			{
				m_iIndex = 0;
				m_pVector = TNULL;
			}

			Iterator(const Iterator& other)
			{
				m_pVector = other.m_pVector;
				m_iIndex = 0;
			}

			Iterator(T2DynamicVector& a_pVector)
			{
				m_iIndex = 0;
				m_pVector = &a_pVector;
			}

			Iterator(TINT a_iIndex, T2DynamicVector& a_pVector)
			{
				m_iIndex = a_iIndex;
				m_pVector = &a_pVector;
			}

			void SetCurrentIndex(TINT a_iIndex) const
			{
				TASSERT(m_pVector);
				TASSERT(a_iIndex < m_pVector->Size());
				m_iIndex = a_iIndex;
			}

			TINT GetCurrentIndex() const
			{
				return m_iIndex;
			}

			TINT& IncrementSafe()
			{
				m_iIndex++;
				TASSERT(m_pVector);

				if (m_pVector->m_iNumElements <= m_iIndex || m_iIndex == 0)
					m_iIndex = -1;

				return m_iIndex;
			}

			void Push(const T& element)
			{
				TASSERT(m_pVector);
				m_pVector->Push(element);
			}

			TBOOL IsOver() const
			{
				return *this == m_pVector->End();
			}

			T& Current()
			{
				TASSERT(m_iIndex >= 0);
				TASSERT(m_pVector);
				TASSERT(m_iIndex < m_pVector->m_iNumElements);
				return m_pVector->At(m_iIndex);
			}

			const T& Current() const
			{
				TASSERT(m_iIndex >= 0);
				TASSERT(m_pVector);
				TASSERT(m_iIndex < m_pVector->m_iNumElements);
				return m_pVector->At(m_iIndex);
			}

			T* operator->()
			{
				return &Current();
			}

			const T* operator->() const
			{
				return &Current();
			}

			TBOOL operator==(const Iterator& a_rOther) const
			{
				return m_iIndex == a_rOther.m_iIndex && m_pVector == a_rOther.m_pVector;
			}

			TBOOL operator!=(const Iterator& a_rOther) const
			{
				return m_iIndex != a_rOther.m_iIndex || m_pVector != a_rOther.m_pVector;
			}

			Iterator& operator++()
			{
				m_iIndex++;
				return *this;
			}

			Iterator operator++(TINT)
			{
				Iterator temp = *this;
				m_iIndex++;
				return temp;
			}

			Iterator& operator--()
			{
				m_iIndex--;
				return *this;
			}

			Iterator operator--(TINT)
			{
				Iterator temp = *this;
				m_iIndex--;
				return temp;
			}

			Iterator operator+(TINT a_iValue) const
			{
				return Iterator(m_iIndex + a_iValue, m_pVector);
			}

			Iterator operator-(TINT a_iValue) const
			{
				return Iterator(m_iIndex - a_iValue, m_pVector);
			}

		private:
			TINT m_iIndex;              // 0x0
			T2DynamicVector* m_pVector; // 0x4
		};

	public:
		constexpr T2DynamicVector(T2Allocator* a_pAllocator, TINT a_iMaxSize, TINT a_iGrowSize) :
			T2GenericDynamicVector(a_pAllocator, a_iMaxSize, a_iGrowSize, sizeof(T))
		{ }

		constexpr T2DynamicVector(T* a_poElements, TINT a_iMaxSize, TINT a_iNumElements) :
			T2GenericDynamicVector(a_poElements, a_iMaxSize, a_iNumElements)
		{ }

		~T2DynamicVector()
		{
			Clear();

			if (m_pAllocator)
			{
				Reallocate(0, sizeof(T));
			}
			else
			{
				m_poElements = TNULL;
			}
		}

		void Clear()
		{
			for (TINT i = 0; i < m_iNumElements; i++)
			{
				((T*)m_poElements + i)->~T();
			}

			m_iNumElements = 0;
		}

		void Reserve(TINT a_iSize)
		{
			if (a_iSize > m_iMaxSize)
			{
				Reallocate(a_iSize, sizeof(T));
			}
		}

		T* PushBack(const T& element)
		{
			Grow(1, sizeof(T));
			return new ((T*)m_poElements + m_iNumElements++) T(element);
		}

		Iterator Begin()
		{
			return Iterator{ 0, *this };
		}

		Iterator End()
		{
			return Iterator{ m_iNumElements, *this };
		}

		Iterator Back()
		{
			TASSERT(m_iNumElements > 0);
			return Iterator{ m_iNumElements - 1, *this };
		}

		TINT Size() const
		{
			return m_iNumElements;
		}

		T& At(TINT a_iIndex)
		{
			TASSERT(a_iIndex < m_iNumElements);
			return *(TREINTERPRETCAST(T*, m_poElements) + a_iIndex);
		}

		const T& At(TINT a_iIndex) const
		{
			TASSERT(a_iIndex < m_iNumElements);
			return *(TREINTERPRETCAST(T*, m_poElements) + a_iIndex);
		}

		T& operator[](TINT a_iIndex)
		{
			return At(a_iIndex);
		}

		const T& operator[](TINT a_iIndex) const
		{
			return At(a_iIndex);
		}
	};

}


