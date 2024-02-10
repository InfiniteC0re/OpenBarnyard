#pragma once

namespace Toshi {
	
	template <class T, TINT Capacity>
	class T2Array
	{
	public:
		void InitialiseAll(const T& a_oValue)
		{
			for (TINT i = 0; i < Capacity; i++)
			{
				this->operator[](i) = a_oValue;
			}
		}

		TINT GetCapacity() const
		{
			return Capacity;
		}

		T* GetArray() const
		{
			return TREINTERPRETCAST(T*, m_pData);
		}

		T& operator[](TINT a_iIndex)
		{
			TASSERT(a_iIndex >= 0 && a_iIndex < Capacity);
			return *(TREINTERPRETCAST(T*, m_pData) + a_iIndex);
		}

		const T& operator[](TINT a_iIndex) const
		{
			TASSERT(a_iIndex >= 0 && a_iIndex < Capacity);
			return *(TREINTERPRETCAST(T*, m_pData) + a_iIndex);
		}

	private:
		TCHAR m_pData[sizeof(T) * Capacity];
	};

}