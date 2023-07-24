#pragma once

namespace Toshi {
	
	template <class T, int Capacity>
	class T2Array
	{
	public:
		void InitialiseAll(const T& a_oValue)
		{
			for (int i = 0; i < Capacity; i++)
			{
				this->operator[](i) = a_oValue;
			}
		}

		int GetCapacity() const
		{
			return Capacity;
		}

		T* GetArray() const
		{
			return TREINTERPRETCAST(T*, m_pData);
		}

		T& operator[](int a_iIndex)
		{
			TASSERT(a_iIndex >= 0 && a_iIndex < Capacity);
			return *(TREINTERPRETCAST(T*, m_pData) + a_iIndex);
		}

		const T& operator[](int a_iIndex) const
		{
			TASSERT(a_iIndex >= 0 && a_iIndex < Capacity);
			return *(TREINTERPRETCAST(T*, m_pData) + a_iIndex);
		}

	private:
		char m_pData[sizeof(T) * Capacity];
	};

}