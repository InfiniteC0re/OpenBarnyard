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
		using Node = T2RedBlackTree<Pair>::Node;

	public:
		T2Map(T2Allocator* a_pAllocator = &T2Allocator::s_GlobalAllocator) : m_RedBlackTree(a_pAllocator)
		{

		}

		~T2Map()
		{
			Clear();
		}

		void Clear()
		{
			m_RedBlackTree.DeleteAll();
		}

		ValueType* Insert(const KeyType& key, const ValueType& value)
		{
			Node* result = TNULL;
			m_RedBlackTree.Insert(result, { key, value });

			return &result->GetValue()->GetSecond();
		}

		void Remove(const KeyType& key)
		{
			Node* result;
			m_RedBlackTree.Find(result, { key });

			TASSERT(result != End());
			m_RedBlackTree.Delete(result);
		}

		void RemoveNode(Node* a_pNode)
		{
			TASSERT(TNULL != a_pNode && a_pNode != End());
			m_RedBlackTree.Delete(a_pNode);
		}

		ValueType* Find(const KeyType& key)
		{
			Node* result;
			m_RedBlackTree.Find(result, { key });

			return result ? &result->GetValue()->GetSecond() : &m_RedBlackTree.End()->GetSecond();
		}

		Node* FindNode(const KeyType& key)
		{
			Node* result;
			m_RedBlackTree.Find(result, { key });

			return result;
		}

		Node* FindNext(Node* a_pNode, const KeyType& a_rKey)
		{
			Node* result;
			m_RedBlackTree.FindNext(result, a_pNode, { a_rKey });

			return result ? &result->GetValue()->GetSecond() : &m_RedBlackTree.End()->GetSecond();
		}

		Node* FindNextNode(Node* a_pNode, const KeyType& a_rKey)
		{
			Node* result;
			m_RedBlackTree.FindNext(result, a_pNode, { a_rKey });

			return result;
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