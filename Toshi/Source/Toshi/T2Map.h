#pragma once
#include "T2Pair.h"
#include "T2RedBlackTree.h"
#include "Toshi/TComparator.h"

TOSHI_NAMESPACE_START

template <class KeyType, class ValueType, class Comparator = TComparator<KeyType>>
class T2Map
{
public:
	using Pair      = T2Pair<KeyType, ValueType, Comparator>;
	using Tree      = T2RedBlackTree<Pair, typename Pair::Comparator>;
	using Iterator  = Tree::Iterator;
	using CIterator = Tree::CIterator;
	using Node      = Tree::Node;

public:
	T2Map( T2Allocator* a_pAllocator = GetGlobalAllocator() )
	    : m_RedBlackTree( a_pAllocator )
	{
	}

	~T2Map()
	{
		Clear();
	}

	// NOTE: retail erases the range [Begin(), End()) of the tree, which deletes every node as well
	// ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00582110
	// $Barnyard: FUNCTION 00582110
	// AInputMap (T2Map<Doodad, CommandBinding>) 00596b70, AInputMapManager maps (<Doodad, TPString8> and <AInputCommand, Command>) 0059adc0
	// $Barnyard: FUNCTION 00596b70
	// $Barnyard: FUNCTION 0059adc0
	// Instantiations: T2Map<TPString8, TINT/TSIZE> 005a5c00 (AInteractionManager, ASoundManager), T2Map<TPString8, TUINT> 005d1e70 (AUnitFlagSetsManager)
	// $Barnyard: FUNCTION 005a5c00
	// $Barnyard: FUNCTION 005d1e70
	// Instantiations: T2Map<TPString8, ...> 00607270 (AKeyFrameLibraryManager), T2Map<TPString8, AModel*> 00612ef0 (AModelRepos),
	// T2Map<TPString8, ...> 00614520 (AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00607270
	// $Barnyard: FUNCTION 00612ef0
	// $Barnyard: FUNCTION 00614520
	void Clear()
	{
		m_RedBlackTree.Erase( m_RedBlackTree.Begin().GetNode(), m_RedBlackTree.End().GetNode() );
	}

	TSIZE Size() const
	{
		return m_RedBlackTree.Size();
	}

	template <class... Args>
	ValueType* Emplace( const KeyType& key, Args&&... args )
	{
		Pair     pair{ key, ValueType( std::forward<Args>( args )... ) };
		Iterator result = m_RedBlackTree.Insert( std::move( pair ) );
		return &result.GetValue()->GetSecond();
	}

	// Instantiations: T2Map<TPString8, TINT/TKeyframeLibrary*> 00606ef0 (AKeyFrameLibraryManager, AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00606ef0
	// ANamedAnimationSet map (T2Map<TPString8, ANamedAnimation*>) 00581550, ANamedAnimationTransitionSet map (T2Map<TPString8, ANamedAnimationTransition>) 00581f30
	// $Barnyard: FUNCTION 00581550
	// $Barnyard: FUNCTION 00581f30
	// AInputMap (T2Map<Doodad, CommandBinding>) 00596af0, AInputMapManager maps (<Doodad, ActionId>) 00597460
	// $Barnyard: FUNCTION 00596af0
	// $Barnyard: FUNCTION 00597460
	// T2Map<TPString8, TINT/TSIZE> 005a5560 (AInteractionManager, ASoundManager), T2Map<TPString8, TUINT> 005d1c60 (AUnitFlagSetsManager), T2Map<TPString8, AWaveBank*> 005dcd00
	// $Barnyard: FUNCTION 005a5560
	// $Barnyard: FUNCTION 005d1c60
	// $Barnyard: FUNCTION 005dcd00
	// T2Map<TINT, ASound*/ASoundLegacy*> 005d8120 (ASoundManager)
	// $Barnyard: FUNCTION 005d8120
	// T2Map<TPString8, AModel*> 00612a30 (AModelRepos), T2Map<TPString8, ...> 00614970 (AMaterialLibraryManager)
	// $Barnyard: FUNCTION 00612a30
	// $Barnyard: FUNCTION 00614970
	ValueType* Insert( const KeyType& key, const ValueType& value )
	{
		Pair     pair{ key, value };
		Iterator result = m_RedBlackTree.Insert( std::move( pair ) );
		return &result.GetValue()->GetSecond();
	}

	ValueType* Insert( const KeyType& key, ValueType&& value )
	{
		Pair     pair{ key, std::move( value ) };
		Iterator result = m_RedBlackTree.Insert( std::move( pair ) );
		return &result.GetValue()->GetSecond();
	}

	void FindAndRemove( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.Find( { key } );

		TASSERT( result != End() );
		m_RedBlackTree.Delete( result.GetNode() );
	}

	void FindLastAndRemove( const KeyType& key )
	{
		Iterator result = m_RedBlackTree.FindLast( { key } );

		TASSERT( result != End() );
		m_RedBlackTree.Delete( result.GetNode() );
	}

	void Remove( Iterator& it )
	{
		TASSERT( it != End() );
		m_RedBlackTree.Delete( it.GetNode() );
	}

	Iterator FindByValue( const ValueType& value )
	{
		for ( auto it = Begin(); it != End(); it++ )
		{
			if ( it.GetValue()->GetSecond() == value )
			{
				return it;
			}
		}

		return End();
	}

	Iterator Find( const KeyType& key )
	{
		return m_RedBlackTree.Find( { key } );
	}

	Iterator FindLast( const KeyType& key )
	{
		return m_RedBlackTree.FindLast( { key } );
	}

	Iterator FindNext( Iterator a_oIterator, const KeyType& a_rKey )
	{
		return m_RedBlackTree.FindNext( a_oIterator.GetNode(), { a_rKey } );
	}

	TBOOL IsValid( Iterator a_oIterator ) const
	{
		return a_oIterator != End();
	}

	Iterator Begin()
	{
		return m_RedBlackTree.Begin();
	}

	Iterator End()
	{
		return m_RedBlackTree.End();
	}

	const CIterator Begin() const
	{
		return m_RedBlackTree.Begin();
	}

	const CIterator End() const
	{
		return m_RedBlackTree.End();
	}

	TBOOL IsEmpty() const
	{
		return Begin() == End();
	}

	Iterator operator[]( const KeyType& key )
	{
		auto it = m_RedBlackTree.Find( { key } );
		TASSERT( it != End() );

		return it;
	}

	T2Allocator* GetAllocator() const
	{
		return m_RedBlackTree.GetAllocator();
	}

private:
	Tree m_RedBlackTree;
};

TOSHI_NAMESPACE_END
