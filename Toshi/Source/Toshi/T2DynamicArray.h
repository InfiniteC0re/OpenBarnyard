#pragma once

namespace Toshi {

	template <class T>
	class T2GenericDynamicArray
	{
	public:
		class Iterator
		{
		public:
			friend T2GenericDynamicArray;
			using Storage = T2GenericDynamicArray;

		public:
			Iterator()
			{
				m_iIndex = 0;
				m_poArray = TNULL;
			}

			Iterator(const Iterator& other)
			{
				m_poArray = other.m_poArray;
				m_iIndex = 0;
			}

			Iterator(Storage& a_poArray)
			{
				m_iIndex = 0;
				m_poArray = &a_poArray;
			}

			Iterator(TINT a_iIndex, Storage& a_poArray)
			{
				m_iIndex = a_iIndex;
				m_poArray = &a_poArray;
			}

			void SetCurrentIndex(TINT a_iIndex) const
			{
				TASSERT(m_poArray);
				TASSERT(a_iIndex < m_poArray->Size());
				m_iIndex = a_iIndex;
			}

			TINT GetCurrentIndex() const
			{
				return m_iIndex;
			}

			TINT& IncrementSafe()
			{
				m_iIndex++;
				TASSERT(m_poArray);

				if (m_poArray->m_iNumElements <= m_iIndex || m_iIndex == 0)
					m_iIndex = -1;

				return m_iIndex;
			}

			void Push(const T& element)
			{
				TASSERT(m_poArray);
				m_poArray->Push(element);
			}

			T* operator->() const
			{
				TASSERT(m_iIndex >= 0);
				TASSERT(m_poArray);
				TASSERT(m_iIndex < m_poArray->m_iNumElements);
				return &m_poArray->m_pData[m_iIndex];
			}

			T& Get()
			{
				TASSERT(m_iIndex >= 0);
				TASSERT(m_poArray);
				TASSERT(m_iIndex < m_poArray->m_iNumElements);
				return m_poArray->m_pData[m_iIndex];
			}

			bool IsOver() const
			{
				return (m_iIndex >= m_poArray->Size() || m_iIndex == -1);
			}

			const T& Get() const
			{
				TASSERT(m_iIndex >= 0);
				TASSERT(m_poArray);
				TASSERT(m_iIndex < m_poArray->m_iNumElements);
				return m_poArray->m_pData[m_iIndex];
			}

			TBOOL operator==(const Iterator& a_rIt) const
			{
				return (m_iIndex == a_rIt.m_iIndex) && (m_poArray == a_rIt.m_poArray);
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
				return Iterator(m_iIndex + a_iValue, m_poArray);
			}

			Iterator operator-(TINT a_iValue) const
			{
				return Iterator(m_iIndex - a_iValue, m_poArray);
			}

		private:
			TINT m_iIndex;      // 0x0
			Storage* m_poArray; // 0x4
		};

	public:
		T2GenericDynamicArray(TINT a_iGrowSize, TINT a_iSize)
		{
			m_iGrowSize = a_iGrowSize;
			m_iNumAllocElements = a_iSize;
			m_iNumElements = 0;

			if (m_iNumAllocElements > 0)
			{
				m_pData = TSTATICCAST(T*, TMemalign(m_iNumAllocElements * sizeof(T), alignof(T)));
			}
			else
			{
				TASSERT(m_iGrowSize != 0);
				m_pData = TNULL;
			}
		}

		~T2GenericDynamicArray()
		{
			if (m_pData)
			{
				TFree(m_pData);
			}
		}

		void Resize(TINT a_iNewSize)
		{
			if (a_iNewSize != 0)
			{
				T* pNewBuffer = TSTATICCAST(T*, TMemalign(a_iNewSize * sizeof(T), alignof(T)));
				size_t uiCopySize = TMath::Min(m_iNumElements, a_iNewSize);

				TUtil::MemCopy(pNewBuffer, m_pData, sizeof(T) * uiCopySize);

				m_iNumAllocElements = a_iNewSize;
				TASSERT(m_iNumElements <= m_iNumAllocElements);

				if (m_pData) TFree(m_pData);
				m_pData = pNewBuffer;
			}
			else
			{
				if (m_pData) TFree(m_pData);
				m_pData = TNULL;
				m_iNumAllocElements = 0;
				m_iNumElements = 0;
			}
		}

		void Clear()
		{
			if (m_iNumAllocElements < 0)
			{
				if (m_pData)
				{
					TFree(m_pData);
					m_pData = TNULL;
				}

				m_iNumAllocElements = 0;
			}

			m_iNumElements = 0;
		}

		Iterator Begin()
		{
			return Iterator((m_iNumElements == 0) ? -1 : 0, *this);
		}

		Iterator End()
		{
			return Iterator(-1, *this);
		}

		TINT Size() const
		{
			return m_iNumElements;
		}

		void SetSize(TINT a_iSize)
		{
			m_iNumElements = a_iSize;
		}

		TINT SizeAllocated() const
		{
			return m_iNumAllocElements;
		}

		TINT GetGrowSize() const
		{
			return m_iGrowSize;
		}

		void SetGrowSize(TINT a_iGrowSize)
		{
			m_iGrowSize = a_iGrowSize;
		}

		T* Push(const T& element)
		{
			GrowBy(1);
			return new (&m_pData[m_iNumElements++]) T(element);
		}

		T& Pop()
		{
			TASSERT(m_iNumElements >= 1);
			return m_pData[--m_iNumElements];
		}

		T& operator[](TINT a_iIndex)
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

		const T& operator[](TINT a_iIndex) const
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

	private:
		void GrowBy(TINT a_iGrowBy)
		{
			if (m_iNumAllocElements < m_iNumElements + a_iGrowBy)
			{
				TASSERT(m_iGrowSize != 0);

				auto iNewSize = TMath::Max(m_iNumAllocElements + m_iGrowSize, m_iNumElements + a_iGrowBy);
				Resize(iNewSize);
			}
		}

	private:
		TINT m_iGrowSize;         // 0x0
		TINT m_iNumElements;      // 0x4
		TINT m_iNumAllocElements; // 0x8
		T* m_pData;               // 0xC
	};

	template <class T>
	class T2DynamicArray : public T2GenericDynamicArray<T>
	{
	public:
		T2DynamicArray(TINT a_iGrowSize = 10, TINT a_iSize = 0) :
			T2GenericDynamicArray<T>(a_iGrowSize, a_iSize)
		{
			
		}
	};

}