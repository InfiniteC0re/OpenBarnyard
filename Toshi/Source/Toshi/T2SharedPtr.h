#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: custom shared pointer class that manages object's lifecycle
// by monitoring for number of existing references.
// Note: this wasn't in the original engine.
//-----------------------------------------------------------------------------
template < typename T >
class T2SharedPtr
{
private:
    struct SharedPtrData_t
    {
#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
        TUINT32 uiMagic;
        void*   pObjectRaw;
#endif // TOSHI2_ENABLE_SHARED_PTR_VALIDATION

        TUINT uiNumRefs;
    };

#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
    static constexpr TUINT32 VALIDATION_MAGIC = 'T2SP';
#endif // TOSHI2_ENABLE_SHARED_PTR_VALIDATION

public:
    // Constructor
    T2SharedPtr() :
        m_pRawPtr( TNULL )
    {}

    // Copy constructor
    T2SharedPtr( const T2SharedPtr& a_rOther )
    {
        m_pRawPtr = a_rOther.m_pRawPtr;

        if ( m_pRawPtr )
            IncrementNumRefs();
    }

    // Move constructor
    T2SharedPtr( T2SharedPtr&& a_rOther )
    {
        m_pRawPtr          = a_rOther.m_pRawPtr;
        a_rOther.m_pRawPtr = TNULL;

        if ( m_pRawPtr )
            IncrementNumRefs();
    }

    // Destructor
    ~T2SharedPtr()
    {
        if ( m_pRawPtr )
            DecrementNumRefs();
    }

    // Returns TTRUE if not TNULL
    TBOOL IsValid() const
    {
        return m_pRawPtr;
    }

    T*       Get() { return TREINTERPRETCAST( T*, m_pRawPtr ); }
    const T* Get() const { return TREINTERPRETCAST( const T*, m_pRawPtr ); }

    operator T*() { return Get(); }
    operator const T*() const { return Get(); }

    T*       operator->() { return Get(); }
    const T* operator->() const { return Get(); }

    T2SharedPtr& operator=( const T2SharedPtr& a_rOther )
    {
        DebugValidate();

        if ( m_pRawPtr )
            DecrementNumRefs();

        m_pRawPtr = a_rOther.m_pRawPtr;

        if ( m_pRawPtr )
            IncrementNumRefs();
    }

    // Use this to create shared pointer objects
    template < class... Args >
    static T2SharedPtr New( Args&&... args )
    {
        // Allocate memory
        TCHAR* pMem    = (TCHAR*)TMalloc( sizeof( T ) + sizeof( SharedPtrData_t ) );
        T*     pObject = new ( pMem + sizeof( SharedPtrData_t ) ) T( std::forward< Args >( args )... );

        // Create shared pointer object
        T2SharedPtr sharedPointer;
        sharedPointer.m_pRawPtr = TREINTERPRETCAST( TCHAR*, pObject );

        // Set shared pointer data data
        SharedPtrData_t* pPtrData = sharedPointer.AccessPtrData();

        pPtrData->uiNumRefs = 1;

#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
        // This can be used to make sure the object was actually created with T2SharedPtr::New
        pPtrData->uiMagic    = VALIDATION_MAGIC;
        pPtrData->pObjectRaw = pObject;
#endif // TOSHI2_ENABLE_SHARED_PTR_VALIDATION

        return sharedPointer;
    }

private:
    // Returns pointer to an internal structure containing info about the object
    SharedPtrData_t* AccessPtrData() const
    {
        TVALIDPTR( m_pRawPtr );
        return TREINTERPRETCAST( SharedPtrData_t*, m_pRawPtr - sizeof( SharedPtrData_t ) );
    }

    void DebugValidate()
    {
#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
        if ( m_pRawPtr )
        {
            SharedPtrData_t* pPtrData = AccessPtrData();
            TASSERT( pPtrData->uiMagic == VALIDATION_MAGIC && pPtrData->pObjectRaw == m_pRawPtr );
        }
#endif
    }

    void IncrementNumRefs()
    {
        TASSERT( TTRUE == IsValid() );
        DebugValidate();

        AccessPtrData()->uiNumRefs++;
    }

    void DecrementNumRefs()
    {
        TASSERT( TTRUE == IsValid() );
        DebugValidate();

        SharedPtrData_t* pPtrData     = AccessPtrData();
        TUINT            uiOldNumRefs = pPtrData->uiNumRefs--;

        if ( uiOldNumRefs == 1 )
        {
            // This was the only reference left, destroy the object and free memory
            Get()->~T();
            TFree( pPtrData );
        }
    }

private:
    TCHAR* m_pRawPtr;
};

TOSHI_NAMESPACE_END
