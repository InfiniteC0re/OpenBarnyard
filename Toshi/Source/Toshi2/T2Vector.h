#pragma once

namespace Toshi
{
	template<class T, size_t MaxSize>
	class T2Vector
	{
	public:
		class Iterator
		{
		public:
			friend class T2Vector;

		public:
			Iterator(T& item) : m_Item(item) { }

			T* Item()
			{
				return &m_Item;
			}

		private:
			T& m_Item;
		};

	public:
		T2Vector()
		{
			m_Size = 0;
		}

		T* Push()
		{
			return &m_Items[m_Size++];
		}

		void PushBack(const T& item)
		{
			TASSERT(Size() < Capacity());
			m_Items[m_Size++] = item;
		}

		void PopBack()
		{
			TASSERT(Size() > 0);
			m_Size--;

			m_Items[m_Size].~T();
			new (&m_Items[m_Size]) T();
		}

		void Clear()
		{
			for (size_t i = 0; i < Size(); i++)
			{
				m_Items[i].~T();
				new (&m_Items[i]) T();
			}

			m_Size = 0;
		}

		void Erase(Iterator it)
		{
			const size_t itemIndex = it.Item() - Begin();
			TASSERT(itemIndex >= 0 && itemIndex <= Size());

			size_t currentIndex = 0;
			for (size_t i = 0; i < Size(); i++)
			{
				if (i == itemIndex)
				{
					continue;
				}

				m_Items[currentIndex++] = m_Items[i];
			}

			m_Size -= 1;
		}

		Iterator MakeIterator(T& item)
		{
			return Iterator(item);
		}

		size_t Size() const
		{
			return m_Size;
		}

		size_t Capacity() const
		{
			return MaxSize;
		}

		bool IsEmpty() const
		{
			return Size() == 0;
		}

		T& Back()
		{
			TASSERT(Size() > 0);
			return m_Items[m_Size - 1];
		}

		T* Begin()
		{
			return m_Items;
		}

		T* End()
		{
			return Begin() + Capacity() * sizeof(T);
		}

	private:
		size_t m_Size;
		T m_Items[MaxSize];
	};
};