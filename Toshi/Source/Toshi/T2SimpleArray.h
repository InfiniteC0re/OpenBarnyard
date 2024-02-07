#pragma once

namespace Toshi {

	// The name of the class is just a guess since it's possible to find the original name
	// Use this when allocated array has 4 extra bytes containing size of array
	template <class T>
	class T2SimpleArray
	{
	private:
		struct AllocateData
		{
			size_t m_uiSize;
		};

	public:
		T2SimpleArray()
		{
			m_pArray = TNULL;
		}

		~T2SimpleArray()
		{
			Destroy();
		}

		void Create(size_t a_uiSize)
		{
			size_t uiAllocDataSize = sizeof(AllocateData) + sizeof(T) * a_uiSize;
			
			AllocateData* pAllocData = TSTATICCAST(AllocateData*, TMalloc(uiAllocDataSize));
			T* pArrayData = TREINTERPRETCAST(T*, pAllocData + 1);

			if (pAllocData)
			{
				pAllocData->m_uiSize = a_uiSize;
				new (pArrayData) T[a_uiSize];

				m_pArray = pArrayData;
			}
			else
			{
				m_pArray = TNULL;
			}
		}

		void Destroy()
		{
			if (m_pArray)
			{
				for (size_t i = 0; i < GetSize(); i++)
				{
					m_pArray[i].~T();
				}

				TFree(GetAllocatedData());
				m_pArray = TNULL;
			}
		}

		T* GetArray()
		{
			return m_pArray;
		}

		size_t GetSize() const
		{
			return GetAllocatedData()->m_uiSize;
		}

		T& operator[](size_t a_uiIndex)
		{
			return m_pArray[a_uiIndex];
		}

		const T& operator[](size_t a_uiIndex) const
		{
			return m_pArray[a_uiIndex];
		}

	private:
		AllocateData* GetAllocatedData() const
		{
			return TREINTERPRETCAST(AllocateData*, TREINTERPRETCAST(uintptr_t, m_pArray) - sizeof(AllocateData));
		}

	private:
		T* m_pArray;
	};

}