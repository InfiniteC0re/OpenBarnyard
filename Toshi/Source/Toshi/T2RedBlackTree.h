#pragma once
#include "Toshi/TComparator.h"
#include "T2Allocator.h"

namespace Toshi
{
class T2GenericRedBlackTreeNode
{
public:
    friend class T2GenericRedBlackTree;

    template < class T >
    friend class T2RedBlackTree;

protected:
    constexpr T2GenericRedBlackTreeNode()
    {
        red       = 0;
        m_pLeft   = this;
        m_pRight  = this;
        m_pParent = this;
    }

    T2GenericRedBlackTreeNode( T2GenericRedBlackTreeNode* pNil )
    {
        red       = 0;
        m_pLeft   = pNil;
        m_pRight  = pNil;
        m_pParent = pNil;
    }

protected:
    TINT                       red;
    T2GenericRedBlackTreeNode* m_pLeft;
    T2GenericRedBlackTreeNode* m_pRight;
    T2GenericRedBlackTreeNode* m_pParent;
};

class T2GenericRedBlackTree
{
protected:
    T2GenericRedBlackTree( T2Allocator* pAllocator ) :
        m_oRoot( ms_oNil )
    {
        m_pAllocator   = pAllocator;
        m_iNumElements = 0;
    }

    ~T2GenericRedBlackTree()
    {
        TASSERT( m_iNumElements == 0 );
    }

    T2Allocator* GetAllocator() const
    {
        return m_pAllocator;
    }

    size_t GetNumElements() const
    {
        return m_iNumElements;
    }

    T2GenericRedBlackTreeNode* GetFirstNode();
    T2GenericRedBlackTreeNode* Insert( T2GenericRedBlackTreeNode* pNode );
    void                       DeleteFixUp( T2GenericRedBlackTreeNode* pNode );
    T2GenericRedBlackTreeNode* DeleteNode( T2GenericRedBlackTreeNode* pNode );

    void                              LeftRotate( T2GenericRedBlackTreeNode* pNode );
    void                              RightRotate( T2GenericRedBlackTreeNode* pNode );
    static T2GenericRedBlackTreeNode* GetSuccessorOf( T2GenericRedBlackTreeNode* pNode );
    static T2GenericRedBlackTreeNode* GetPredecessorOf( T2GenericRedBlackTreeNode* pNode );

private:
    void CheckValid()
    {
        TASSERT( ms_oNil.red == 0 );
        TASSERT( m_oRoot.red == 0 );
    }

protected:
    static constinit T2GenericRedBlackTreeNode ms_oNil;

protected:
    T2Allocator*              m_pAllocator;
    T2GenericRedBlackTreeNode m_oRoot;
    size_t                    m_iNumElements;
};

template < class T >
class T2RedBlackTreeNode : public T2GenericRedBlackTreeNode
{
public:
    template < class T >
    friend class T2RedBlackTree;

public:
    T2RedBlackTreeNode( const T& value ) :
        m_Value( value )
    {
    }

    T* GetValue()
    {
        return &m_Value;
    }

    TBOOL IsLeftNodeNext( const T& value )
    {
        if ( m_Value == value )
        {
            return TFALSE;
        }
        else
        {
            if ( m_Value < value )
            {
                return TFALSE;
            }

            return TTRUE;
        }
    }

    TBOOL operator==( const T& other ) const
    {
        return TComparator< T >::IsEqual( m_Value, other );
    }

    TBOOL operator==( const T2RedBlackTreeNode< T >& other ) const
    {
        return TComparator< T >::IsEqual( m_Value, other.m_Value );
    }

    TBOOL operator!=( const T& other ) const
    {
        return !TComparator< T >::IsEqual( m_Value, other );
    }

    TBOOL operator!=( const T2RedBlackTreeNode< T >& other ) const
    {
        return !TComparator< T >::IsEqual( m_Value, other.m_Value );
    }

    TBOOL operator>( const T2RedBlackTreeNode< T >& other ) const
    {
        return TComparator< T >::IsGreater( m_Value, other.m_Value );
    }

    TBOOL operator<( const T2RedBlackTreeNode< T >& other ) const
    {
        return TComparator< T >::IsLess( m_Value, other.m_Value );
    }

    TBOOL operator<=( const T2RedBlackTreeNode< T >& other ) const
    {
        return TComparator< T >::IsLessOrEqual( m_Value, other.m_Value );
    }

    TBOOL operator>=( const T2RedBlackTreeNode< T >& other ) const
    {
        return TComparator< T >::IsGreaterOrEqual( m_Value, other.m_Value );
    }

private:
    T m_Value;
};

template < class T >
class T2RedBlackTree : public T2GenericRedBlackTree
{
public:
    using Node = T2RedBlackTreeNode< T >;

    class Iterator
    {
    public:
        TFORCEINLINE Iterator( Node* ppNode ) :
            m_ppNode( ppNode ) {}

        TFORCEINLINE Iterator Next()
        {
            Iterator next = *this;
            m_ppNode      = TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_ppNode ) );
            return next;
        }

        TFORCEINLINE Iterator Prev()
        {
            Iterator prev = *this;
            m_ppNode      = TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_ppNode ) );
            return prev;
        }

        TFORCEINLINE Node* GetNode()
        {
            return m_ppNode;
        }

        TFORCEINLINE TBOOL operator==( const Node* a_pNode ) const
        {
            return m_ppNode == a_pNode;
        }

        TFORCEINLINE TBOOL operator==( const Iterator& other ) const
        {
            return m_ppNode == other.m_ppNode;
        }

        TFORCEINLINE TBOOL operator==( const T& other ) const
        {
            return m_ppNode == other;
        }

        TFORCEINLINE T& operator*() const
        {
            return m_ppNode->m_Value;
        }

        TFORCEINLINE T* operator->() const
        {
            return &m_ppNode->m_Value;
        }

        TFORCEINLINE Iterator operator++( TINT )
        {
            return Next();
        }

        TFORCEINLINE Iterator& operator++()
        {
            m_ppNode = TSTATICCAST( Node, T2RedBlackTree::GetSuccessorOf( m_ppNode ) );
            return *this;
        }

        TFORCEINLINE Iterator operator--( TINT )
        {
            return Prev();
        }

        TFORCEINLINE Iterator& operator--()
        {
            m_ppNode = TSTATICCAST( Node, T2RedBlackTree::GetPredecessorOf( m_ppNode ) );
            return *this;
        }

    private:
        Node* m_ppNode;
    };

public:
    T2RedBlackTree( T2Allocator* pAllocator = &T2Allocator::s_GlobalAllocator ) :
        T2GenericRedBlackTree( pAllocator )
    {
    }

    TBOOL IsRoot( Node* pNode )
    {
        return pNode == &m_oRoot;
    }

    Iterator Begin()
    {
        return Iterator( TREINTERPRETCAST( Node*, GetFirstNode() ) );
    }

    Iterator End()
    {
        return Iterator( TREINTERPRETCAST( Node*, &m_oRoot ) );
    }

    void DeleteAll()
    {
        while ( GetNumElements() > 0 )
        {
            auto pNode = TSTATICCAST( Node, GetFirstNode() );
            Delete( pNode );
        }
    }

    void Delete( Node* a_pNode )
    {
        m_pAllocator->Delete( DeleteNode( a_pNode ) );
    }

    void Erase( Node* a_pFrom, Node* a_pTo )
    {
        Node* pNode = a_pFrom;

        while ( pNode != a_pTo )
        {
            auto pNext = GetSuccessorOf( pNode );
            Delete( pNode );
            pNode = pNext;
        }
    }

    void Insert( Node*& insertedNode, const T& value )
    {
        Node* pNode     = m_pAllocator->New< Node >( value );
        pNode->m_pLeft  = &ms_oNil;
        pNode->m_pRight = &ms_oNil;

        Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );
        Node* pInsertTo    = TSTATICCAST( Node, &m_oRoot );

        while ( pCurrentNode != &ms_oNil )
        {
            pInsertTo = pCurrentNode;

            if ( pCurrentNode->IsLeftNodeNext( value ) )
            {
                pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pLeft );
            }
            else
            {
                pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pRight );
            }
        }

        pNode->m_pParent = pInsertTo;

        if ( pInsertTo == &m_oRoot || pInsertTo->IsLeftNodeNext( value ) )
        {
            pInsertTo->m_pLeft = pNode;
        }
        else
        {
            pInsertTo->m_pRight = pNode;
        }

        TASSERT( ms_oNil.red == 0, "ms_oNil not red in T2GenericRedBlackTree::TreeInsertHelp" ); // TreeInsertHelp????
        T2GenericRedBlackTree::Insert( pNode );
        insertedNode = pNode;
    }

    Iterator Find( Node*& foundNode, const T& value )
    {
        Node* pCurrentNode = TSTATICCAST( Node, m_oRoot.m_pLeft );

        while ( pCurrentNode != &ms_oNil )
        {
            if ( pCurrentNode->operator==( value ) )
            {
                foundNode = pCurrentNode;
                return foundNode;
            }

            if ( pCurrentNode->IsLeftNodeNext( value ) )
            {
                pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pLeft );
            }
            else
            {
                pCurrentNode = TSTATICCAST( Node, pCurrentNode->m_pRight );
            }
        }

        foundNode = TSTATICCAST( Node, &m_oRoot );
        return foundNode;
    }

    Iterator FindNext( Node*& foundNode, Node*& nextAfter, const T& value )
    {
        Node* pNode = TSTATICCAST( Node, GetSuccessorOf( nextAfter ) );

        if ( pNode->operator==( value ) )
        {
            foundNode = pNode;
            return foundNode;
        }

        foundNode = TSTATICCAST( Node, &m_oRoot );
        return foundNode;
    }

    TSIZE Size() const
    {
        return GetNumElements();
    }
};

} // namespace Toshi