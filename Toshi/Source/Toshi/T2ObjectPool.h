#pragma once
#include "T2Allocator.h"

#undef GetObject

TOSHI_NAMESPACE_START

class TOSHI_API T2GenericObjectPool : protected T2Allocator
{
protected:
    struct UnusedObject
    {
        UnusedObject* pNextObject;
    };

public:
    virtual ~T2GenericObjectPool() = default;

protected:
    virtual void* Malloc( TSIZE a_uiSize, TSIZE a_uiAlignment ) OVERRIDE
    {
        return TNULL;
    }

    virtual void* Malloc( TSIZE a_uiSize ) OVERRIDE
    {
        return GetObject();
    }

    virtual void Free( void* a_pPtr ) OVERRIDE
    {
        ReturnObject( TSTATICCAST( UnusedObject, a_pPtr ) );
    }

    virtual TBOOL CanAllocate( TSIZE size ) const OVERRIDE
    {
        return m_pHead != TNULL;
    }

    virtual void* TryMalloc( TSIZE size, TSIZE alignment ) OVERRIDE
    {
        return TNULL;
    }

    virtual void* TryMalloc( TSIZE size ) OVERRIDE
    {
        if ( m_pHead != TNULL )
        {
            return GetObject();
        }

        return TNULL;
    }

protected:
    void Initialise( UnusedObject* a_pObjects, TUINT a_uiMaxNumber, TUINT a_uiObjectSize )
    {
        TASSERT( a_uiMaxNumber > 1 );
        m_pHead               = a_pObjects;
        UnusedObject* pObject = a_pObjects;

        for ( TUINT i = a_uiMaxNumber - 1; i != 0; i-- )
        {
            UnusedObject* pNext  = TREINTERPRETCAST( UnusedObject*, TREINTERPRETCAST( TUINTPTR, pObject ) + a_uiObjectSize );
            pObject->pNextObject = pNext;
            pObject              = pNext;
        }

        pObject->pNextObject = TNULL;
    }

    UnusedObject* GetObject()
    {
        UnusedObject* pNode = m_pHead;
        m_pHead             = m_pHead->pNextObject;
        return pNode;
    }

    void ReturnObject( UnusedObject* a_pObject )
    {
        a_pObject->pNextObject = m_pHead;
        m_pHead                = a_pObject;
    }

    TUINT GetNumFreeObjects() const
    {
        TUINT uiNumber = 0;
        for ( auto it = m_pHead; it != TNULL; it = it->pNextObject, uiNumber++ )
            ;

        return uiNumber;
    }

protected:
    UnusedObject* m_pHead;
};

template < class T, TUINT MaxNumber, TUINT ObjectSize = sizeof( T ), TUINT Alignment = alignof( T ) >
class T2ObjectPool :
    protected T2GenericObjectPool
{
public:
    TSTATICASSERT( MaxNumber >= 2 );
    TSTATICASSERT( sizeof( T ) >= sizeof( T2GenericObjectPool::UnusedObject ) );

public:
    T2ObjectPool()
    {
        T2GenericObjectPool::Initialise(
            TREINTERPRETCAST( T2GenericObjectPool::UnusedObject*, GetObjects() ),
            MaxNumber,
            ObjectSize );
    }

    template < class... Args >
    T* NewObject( Args&&... args )
    {
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( ObjectSize ) );
        T* pValue = new ( Malloc( ObjectSize ) ) T( std::forward< Args >( args )... );
        return pValue;
    }

    T* AllocateObject()
    {
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( ObjectSize ) );
        return TSTATICCAST( T, Malloc( ObjectSize ) );
    }

    void FreeObject( T* a_pObject )
    {
        TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
        Free( a_pObject );
    }

    void DeleteObject( T* a_pObject )
    {
        TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
        a_pObject->~T();
        Free( a_pObject );
    }

    virtual TSIZE GetUsedSize() const OVERRIDE
    {
        return ( MaxNumber - GetNumFreeObjects() ) * ObjectSize;
    }

    virtual TSIZE GetCapacity() const OVERRIDE
    {
        return MaxNumber * ObjectSize;
    }

    virtual TBOOL IsAddressInPool( const void* a_pAddress ) const
    {
        return ( TREINTERPRETCAST( TUINTPTR, this ) + sizeof( T2GenericObjectPool ) ) <= TREINTERPRETCAST( TUINTPTR, a_pAddress ) &&
            TREINTERPRETCAST( TUINTPTR, a_pAddress ) < ( TREINTERPRETCAST( TUINTPTR, this ) + sizeof( T2GenericObjectPool ) + ( MaxNumber * ObjectSize ) );
    }

    TBOOL CanAllocate() const { return T2GenericObjectPool::CanAllocate( ObjectSize ); }

private:
    T* GetObjects()
    {
        return TREINTERPRETCAST( T*, m_aObjects );
    }

    const T* GetObjects() const
    {
        return TREINTERPRETCAST( const T*, m_aObjects );
    }

private:
    TBYTE m_aObjects[ MaxNumber * ObjectSize ];
};

template < class T >
class T2DynamicObjectPool :
    protected T2GenericObjectPool
{
public:
    T2DynamicObjectPool( T2Allocator* a_pAllocator, TINT a_iMaxNumber )
    {
        TVALIDPTR( a_pAllocator );
        m_iMaxNumber = a_iMaxNumber;
        m_pAllocator = a_pAllocator;

        TUINT32 uiClassSize      = sizeof( T );
        TUINT32 uiClassAlignment = alignof( T );
        m_uiObjectSize           = ( uiClassSize - 1 ) + uiClassAlignment & ~( uiClassAlignment - 1U );

        TASSERT( m_uiObjectSize >= sizeof( T ) && uiClassAlignment > 0 );

        if ( m_iMaxNumber < 1 )
        {
            m_pData = TNULL;
        }
        else
        {
            m_pData = m_pAllocator->Malloc( m_iMaxNumber * m_uiObjectSize, uiClassAlignment );
        }

        TVALIDPTR( m_pData );
        T2GenericObjectPool::Initialise(
            TREINTERPRETCAST( T2GenericObjectPool::UnusedObject*, m_pData ),
            m_iMaxNumber,
            m_uiObjectSize );
    }

    virtual ~T2DynamicObjectPool()
    {
        m_pAllocator->Free( m_pData );
    }

    template < class... Args >
    T* NewObject( Args&&... args )
    {
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
        T* pValue = new ( Malloc( m_uiObjectSize ) ) T( std::forward< Args >( args )... );
        return pValue;
    }

    T* AllocateObject()
    {
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
        return TSTATICCAST( T, Malloc( m_uiObjectSize ) );
    }

    void FreeObject( T* a_pObject )
    {
        TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
        Free( a_pObject );
    }

    void DeleteObject( T* a_pObject )
    {
        TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
        a_pObject->~T();
        Free( a_pObject );
    }

    virtual TSIZE GetUsedSize() const OVERRIDE
    {
        return ( m_iMaxNumber - GetNumFreeObjects() ) * m_uiObjectSize;
    }

    virtual TSIZE GetCapacity() const OVERRIDE
    {
        return m_iMaxNumber * m_uiObjectSize;
    }

    virtual TBOOL IsAddressInPool( const void* a_pAddress ) const
    {
        return TREINTERPRETCAST( TUINTPTR, m_pData ) <= TREINTERPRETCAST( TUINTPTR, a_pAddress ) &&
            TREINTERPRETCAST( TUINTPTR, a_pAddress ) < TREINTERPRETCAST( TUINTPTR, m_pData ) + ( m_iMaxNumber * m_uiObjectSize );
    }

    TBOOL CanAllocate() const { return T2GenericObjectPool::CanAllocate( m_uiObjectSize ); }

private:
    T2Allocator* m_pAllocator;
    TINT         m_iMaxNumber;
    TUINT        m_uiObjectSize;
    void*        m_pData;
};

template < class TClassType >
class T2DerivedDynamicObjectPool :
    protected T2GenericObjectPool
{
public:
    T2DerivedDynamicObjectPool( T2Allocator* a_pAllocator, TINT a_iMaxNumber )
    {
        TVALIDPTR( a_pAllocator );
        m_iMaxNumber = a_iMaxNumber;
        m_pAllocator = a_pAllocator;

        TUINT32 uiClassSize      = TClassType::GetClassStatic()->GetMaxSizeOfDerivedClasses();
        TUINT32 uiClassAlignment = TClassType::GetClassStatic()->GetMaxAlignmentOfDerivedClasses();
        m_uiObjectSize           = ( uiClassSize - 1 ) + uiClassAlignment & ~( uiClassAlignment - 1U );

        TASSERT( m_uiObjectSize >= sizeof( TClassType ) && uiClassAlignment > 0 );

        if ( m_iMaxNumber < 1 )
        {
            m_pData = TNULL;
        }
        else
        {
            m_pData = m_pAllocator->Malloc( m_iMaxNumber * m_uiObjectSize, uiClassAlignment );
        }

        TVALIDPTR( m_pData );
        T2GenericObjectPool::Initialise(
            TREINTERPRETCAST( T2GenericObjectPool::UnusedObject*, m_pData ),
            m_iMaxNumber,
            m_uiObjectSize );
    }

    virtual ~T2DerivedDynamicObjectPool()
    {
        m_pAllocator->Free( m_pData );
    }

    template < class... Args >
    TClassType* NewObject( Args&&... args )
    {
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
        TClassType* pValue = new ( Malloc( m_uiObjectSize ) ) TClassType( std::forward< Args >( args )... );
        return pValue;
    }

    template < class T, class... Args >
    T* NewDerivedObject( Args&&... args )
    {
        TASSERT( TTRUE == T::GetClassStatic()->IsA( TClassType::GetClassStatic() ) );
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
        T* pValue = new ( Malloc( m_uiObjectSize ) ) T( std::forward< Args >( args )... );
        return pValue;
    }

    TClassType* AllocateObject()
    {
        TASSERT( TTRUE == T2GenericObjectPool::CanAllocate( m_uiObjectSize ) );
        return TSTATICCAST( TClassType, Malloc( m_uiObjectSize ) );
    }

    void FreeObject( TClassType* a_pObject )
    {
        TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
        Free( a_pObject );
    }

    void DeleteObject( TClassType* a_pObject )
    {
        TASSERT( TTRUE == IsAddressInPool( a_pObject ) );
        a_pObject->~TClassType();
        Free( a_pObject );
    }

    virtual TSIZE GetUsedSize() const OVERRIDE
    {
        return ( m_iMaxNumber - GetNumFreeObjects() ) * m_uiObjectSize;
    }

    virtual TSIZE GetCapacity() const OVERRIDE
    {
        return m_iMaxNumber * m_uiObjectSize;
    }

    virtual TBOOL IsAddressInPool( const void* a_pAddress ) const
    {
        return TREINTERPRETCAST( TUINTPTR, m_pData ) <= TREINTERPRETCAST( TUINTPTR, a_pAddress ) &&
            TREINTERPRETCAST( TUINTPTR, a_pAddress ) < TREINTERPRETCAST( TUINTPTR, m_pData ) + ( m_iMaxNumber * m_uiObjectSize );
    }

    TBOOL CanAllocate() const { return T2GenericObjectPool::CanAllocate( m_uiObjectSize ); }

private:
    T2Allocator* m_pAllocator;
    TINT         m_iMaxNumber;
    TUINT        m_uiObjectSize;
    void*        m_pData;
};

TOSHI_NAMESPACE_END
