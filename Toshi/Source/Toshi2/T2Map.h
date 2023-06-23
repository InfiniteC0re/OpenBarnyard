#pragma once
#include "T2Pair.h"
#include "T2RedBlackTree.h"
#include "Toshi/Core/TComparator.h"

namespace Toshi {

	template <class KeyType, class ValueType, class Comparator>
	class T2Map : public T2RedBlackTree<T2Pair<KeyType, ValueType, Comparator>>
	{
	public:
		using Pair = T2Pair<KeyType, ValueType, Comparator>;

	public:
		ValueType* Insert(const KeyType& key, const ValueType& value)
		{
			T2RedBlackTreeNode<Pair>* result = TNULL;
			T2RedBlackTree<Pair>::Insert(result, { key, value });

			return &result->GetValue()->GetSecond();
		}

		ValueType* Find(const KeyType& key)
		{
			T2RedBlackTreeNode<Pair>* result = TNULL;
			T2RedBlackTree<Pair>::Find(result, { key });

			return result ? &result->GetValue()->GetSecond() : TNULL;
		}

		//T2RedBlackTree<Pair>::Iterator Begin()
		//{
		//	return m_RedBlackTree.Begin();
		//}

		//T2RedBlackTree<Pair>::Iterator End()
		//{
		//	return m_RedBlackTree.End();
		//}

	private:
		//T2RedBlackTree<Pair> m_RedBlackTree;
	};

}