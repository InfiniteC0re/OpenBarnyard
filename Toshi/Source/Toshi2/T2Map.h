#pragma once
#include "T2Pair.h"
#include "T2RedBlackTree.h"
#include "Toshi/Core/TComparator.h"

namespace Toshi {

	template <class KeyType, class ValueType, class Comparator = TComparator<KeyType>>
	class T2Map
	{
	public:
		using Pair = T2Pair<KeyType, ValueType, Comparator>;
		using Iterator = T2RedBlackTree<Pair>::Iterator;

	public:
		T2Map(T2Allocator* a_pAllocator = &T2Allocator::s_GlobalAllocator) : m_RedBlackTree(a_pAllocator)
		{

		}

		void Clear()
		{
			m_RedBlackTree.DeleteAll();
		}

		ValueType* Insert(const KeyType& key, const ValueType& value)
		{
			T2RedBlackTreeNode<Pair>* result = TNULL;
			m_RedBlackTree.Insert(result, { key, value });

			return &result->GetValue()->GetSecond();
		}

		void Remove(const KeyType& key)
		{
			T2RedBlackTreeNode<Pair>* result = TNULL;
			m_RedBlackTree.Find(result, { key });

			TASSERT(result != End());
			m_RedBlackTree.Delete(result);
		}

		ValueType* Find(const KeyType& key)
		{
			T2RedBlackTreeNode<Pair>* result = TNULL;
			m_RedBlackTree.Find(result, { key });

			return result ? &result->GetValue()->GetSecond() : &m_RedBlackTree.End()->GetSecond();
		}

		Iterator Begin()
		{
			return m_RedBlackTree.Begin();
		}

		Iterator End()
		{
			return m_RedBlackTree.End();
		}

	private:
		T2RedBlackTree<Pair> m_RedBlackTree;
	};

}