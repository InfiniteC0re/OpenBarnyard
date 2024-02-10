#pragma once
#include <type_traits>

#define T2_FOREACH(vecName, iteratorName) \
    for (auto iteratorName = vecName.Begin(); iteratorName != vecName.End(); iteratorName++)

#define T2_FOREACH_BACK(vecName, iteratorName) \
	for (auto iteratorName = vecName.Tail(); iteratorName != vecName.End(); iteratorName--)

#define T2_FOREACH_ARRAY(arrName, iteratorName) \
    for (TINT iteratorName = 0; iteratorName < TARRAYSIZE(arrName); iteratorName++)

#define T2_FOREACH_ARRAY_BACK(arrName, iteratorName) \
	for (TINT iteratorName = TARRAYSIZE(arrName) - 1; iteratorName >= 0; iteratorName--)

#define T2_DEFINE_ITERATOR_FRIEND() \
	template<class T, class Node> friend class Toshi::T2Iterator

#define T2_DEFINE_ITERATOR(TYPE, NODE_TYPE) \
	using Iterator_t = Toshi::T2Iterator<TYPE, NODE_TYPE>

namespace Toshi {

	template <class T, class Node>
	class T2Iterator
	{
	public:
		TSTATICASSERT(std::is_base_of<Node, T>::value);

		T2Iterator()
		{
			m_pPtr = TNULL;
		}

		T2Iterator(Node* pPtr)
		{
			m_pPtr = static_cast<T*>(pPtr);
		}

		T2Iterator(T* pPtr)
		{
			m_pPtr = pPtr;
		}

		void operator=(const T2Iterator& other)
		{
			m_pPtr = other.m_pPtr;
		}

		void operator=(T* pPtr)
		{
			m_pPtr = pPtr;
		}

		T* operator->() const
		{
			TASSERT(m_pPtr != TNULL);
			return m_pPtr;
		}

		operator T* () const
		{
			TASSERT(m_pPtr != TNULL);
			return static_cast<T*>(m_pPtr);
		}

		T2Iterator operator++(TINT)
		{
			TASSERT(m_pPtr != TNULL);
			T2Iterator old = m_pPtr;
			m_pPtr = static_cast<T*>(m_pPtr->Next());
			return old;
		}

		T2Iterator operator--(TINT)
		{
			TASSERT(m_pPtr != TNULL);
			T2Iterator old = m_pPtr;
			m_pPtr = static_cast<T*>(m_pPtr->Prev());
			return old;
		}

		T2Iterator operator++()
		{
			TASSERT(m_pPtr != TNULL);
			m_pPtr = static_cast<T*>(m_pPtr->Next());
			return T2Iterator{ m_pPtr };
		}

		T2Iterator operator--()
		{
			TASSERT(m_pPtr != TNULL);
			m_pPtr = static_cast<T*>(m_pPtr->Prev());
			return T2Iterator{ m_pPtr };
		}

	private:
		T* m_pPtr;
	};

}