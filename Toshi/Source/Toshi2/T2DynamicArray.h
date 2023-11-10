#pragma once

namespace Toshi {

	template <class T>
	class T2GenericDynamicArray
	{
	public:
		class Iterator
		{
		public:
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

			Iterator(int a_iIndex, Storage& a_poArray)
			{
				m_iIndex = a_iIndex;
				m_poArray = &a_poArray;
			}

			void SetCurrentIndex(int a_iIndex) const
			{
				TASSERT(m_poArray);
				TASSERT(a_iIndex < m_poArray->GetNumElements());
				m_iIndex = a_iIndex;
			}

			int GetCurrentIndex() const
			{
				return m_iIndex;
			}

			int& IncrementSafe()
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
				return (m_iIndex >= m_poArray->GetNumElements() || m_iIndex == -1);
			}

			const T& Get() const
			{
				TASSERT(m_iIndex >= 0);
				TASSERT(m_poArray);
				TASSERT(m_iIndex < m_poArray->m_iNumElements);
				return m_poArray->m_pData[m_iIndex];
			}

			Iterator& operator++()
			{
				m_iIndex++;
				return *this;
			}

			Iterator operator++(int)
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

			Iterator operator--(int)
			{
				Iterator temp = *this;
				m_iIndex--;
				return temp;
			}

			Iterator operator+(int a_iValue) const
			{
				return Iterator(m_iIndex + a_iValue, m_poArray);
			}

			Iterator operator-(int a_iValue) const
			{
				return Iterator(m_iIndex - a_iValue, m_poArray);
			}

		private:
			int m_iIndex;       // 0x0
			Storage* m_poArray; // 0x4
		};

	public:
		T2GenericDynamicArray(int a_iGrowSize, int a_iSize)
		{
			m_iGrowSize = a_iGrowSize;
			m_iNumAllocElements = a_iSize;
			m_iNumElements = 0;

			if (m_iNumAllocElements > 0)
			{
				m_pData = TSTATICCAST(T*, TMemalign(alignof(T), m_iNumAllocElements * sizeof(T)));
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

		void Bind(Iterator& a_rIterator) const
		{
			a_rIterator->m_poArray = this;
			a_rIterator->m_iIndex = (m_iNumElements == 0) ? -1 : 0;
		}

		int GetNumElements() const
		{
			return m_iNumElements;
		}

		int GetNumAllocElements() const
		{
			return m_iNumAllocElements;
		}

		int GetGrowSize() const
		{
			return m_iGrowSize;
		}

		void SetGrowSize(int a_iGrowSize)
		{
			m_iGrowSize = a_iGrowSize;
		}

		void Push(const T& element)
		{
			GrowBy(1);
			(*this)[m_iNumElements++] = element;
		}

		T& operator[](int a_iIndex)
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

		const T& operator[](int a_iIndex) const
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

	private:
		void GrowBy(int a_iGrowBy)
		{
			if (m_iNumAllocElements < m_iNumElements + a_iGrowBy)
			{
				TASSERT(m_iGrowSize != 0);

				auto iNewSize = TMath::Max(m_iNumAllocElements + m_iGrowSize, m_iNumElements + a_iGrowBy);
				Resize(iNewSize);
			}
		}

		void Resize(int a_iNewSize)
		{
			if (a_iNewSize != 0)
			{
				T* pNewBuffer = TSTATICCAST(T*, TMemalign(alignof(T), a_iNewSize * sizeof(T)));
				size_t uiCopySize = TMath::Min(m_iNumElements, a_iNewSize);

				for (size_t i = 0; i < uiCopySize; i++)
					pNewBuffer[i] = m_pData[i];

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

	private:
		int m_iGrowSize;         // 0x0
		int m_iNumElements;      // 0x4
		int m_iNumAllocElements; // 0x8
		T* m_pData;              // 0xC
	};

	template <class T>
	class T2DynamicArray : public T2GenericDynamicArray<T>
	{
	public:
		T2DynamicArray(int a_iGrowSize = 10, int a_iSize = 0) :
			T2GenericDynamicArray<T>(a_iGrowSize, a_iSize)
		{
			
		}
	};

}