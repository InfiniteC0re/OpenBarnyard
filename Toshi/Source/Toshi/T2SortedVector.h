#pragma once
#include "T2Vector.h"
#include "T2Iterator.h"

TOSHI_NAMESPACE_START

// -1 => push right here before a_rcVal2
//  0 => they are equal, add new element after it to preserve original ordering
//  1 => should add somewhere after a_rcVal2, so go to the next node
template <class T>
struct T2SortedVectorDefaultSortResults
{
	TINT operator()( const T& a_rcVal1, const T& a_rcVal2 ) const
	{
		if ( a_rcVal1 < a_rcVal2 )
			return 1;

		return -1;
	}
};

template <typename T, typename Container, typename SortResults = T2SortedVectorDefaultSortResults<T>>
class T2SortedVector : private Container
{
public:
	using Iterator = Container::Iterator;

public:
	template <class... Args>
	T2SortedVector( Args&&... args )
	    : Container( std::forward<Args>( args )... )
	{}

	~T2SortedVector() = default;

	// With a help of binary search algorithm, looks for an insertion place of the specified value
	Iterator FindInsertionPoint( const T& a_rcValue, TBOOL& a_rbNotUnique )
	{
		TINT iSearchStart = 0;
		TINT iSearchEnd   = Container::Size() - 1;
		a_rbNotUnique     = TFALSE;

		while ( iSearchStart <= iSearchEnd )
		{
			TINT iMiddle     = ( iSearchStart + iSearchEnd ) / 2;
			TINT iSortResult = SortResults()( a_rcValue, Container::At( iMiddle ) );

			if ( iSortResult < 0 )
			{
				// Should look for the insertion place at the left side
				iSearchEnd = iMiddle - 1;
			}
			else if ( iSortResult > 0 )
			{
				// Should look for the insertion place at the right side
				iSearchStart = iMiddle + 1;
			}
			else
			{
				// We found an item that is equal to this
				// Now, skip all equal items to prevent order changes
				iSearchStart += 1;

				while ( iSearchStart <= iSearchEnd &&
				        SortResults()( a_rcValue, Container::At( iSearchStart ) ) == 0 )
				{
					iSearchStart += 1;
				}

				a_rbNotUnique = TTRUE;
				break;
			}
		}

		return Iterator( iSearchStart, this );
	}

	Iterator Push( const T& a_rValue )
	{
		TBOOL bNotUnique;
		return Container::InsertBefore( FindInsertionPoint( a_rValue, bNotUnique ), a_rValue );
	}

	Iterator Push( const T* a_pValue ) { return Push( *a_pValue ); }

	void PopBack() { Container::PopBack(); }
	void PopFront() { Container::PopFront(); }

	void Clear() { Container::Clear(); }

	Iterator Find( const T& a_rValue ) { return Container::Find( a_rValue ); }
	void     Erase( const Iterator& a_rIterator ) { Container::Erase( a_rIterator ); }
	void     FindAndErase( const T& a_rcItem ) { Container::FindAndErase( a_rcItem ); }
	void     EraseFast( const Iterator& a_rIterator ) { Container::EraseFast( a_rIterator ); }
	void     FindAndEraseFast( const T& a_rcItem ) { Container::FindAndEraseFast( a_rcItem ); }

	TINT  Size() const { return Container::Size(); }
	TINT  Capacity() const { return Container::Capacity(); }
	TBOOL IsEmpty() const { return Container::IsEmpty(); }

	Iterator Front() { return Container::Front(); }
	Iterator Back() { return Container::Back(); }
	Iterator Begin() { return Container::Begin(); }
	Iterator End() { return Container::End(); }

	T&       At( TINT a_iIndex ) { return Container::At( a_iIndex ); }
	const T& At( TINT a_iIndex ) const { return Container::At( a_iIndex ); }

	T&       operator[]( TINT a_iIndex ) { return Container::At( a_iIndex ); }
	const T& operator[]( TINT a_iIndex ) const { return Container::At( a_iIndex ); }

	Container* AccessContainer() { return TSTATICCAST( Container, this ); }
	Container* operator->() { return TSTATICCAST( Container, this ); }
};

TOSHI_NAMESPACE_END
