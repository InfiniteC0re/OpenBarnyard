#pragma once

TOSHI_NAMESPACE_START

template <typename T>
class T2WeakPtr;

//-----------------------------------------------------------------------------
// Purpose: custom shared pointer class that manages object's lifecycle
// by monitoring for number of existing references.
// Note: this wasn't in the original engine.
//-----------------------------------------------------------------------------
template <typename T>
class T2SharedPtr
{
private:
	struct SharedPtrData_t
	{
#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
		TUINT32 uiMagic;
		void*   pObjectRaw;
#endif // TOSHI2_ENABLE_SHARED_PTR_VALIDATION

		TUINT uiNumWeakRefs;
		TUINT uiNumRefs;
	};

	template <typename T> friend class T2WeakPtr;

#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
	static constexpr TUINT32 VALIDATION_MAGIC = 'T2SP';
#endif // TOSHI2_ENABLE_SHARED_PTR_VALIDATION

public:
	// Constructor
	T2SharedPtr()
	    : m_pRawPtr( TNULL )
	{}

	T2SharedPtr( T2WeakPtr<T>& a_rWeakPtr )
	    : m_pRawPtr( a_rWeakPtr.Get() )
	{
		if ( m_pRawPtr )
			IncrementNumRefs();
	}

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
	}

	// Destructor
	~T2SharedPtr()
	{
		if ( m_pRawPtr )
			DecrementNumRefs();
	}

	// Returns TTRUE if set
	TBOOL IsValid() const
	{
		return m_pRawPtr;
	}

	static TBOOL IsValid( T* a_pRawPtr )
	{
		return a_pRawPtr;
	}

	void Reset()
	{
		if ( m_pRawPtr )
		{
			DecrementNumRefs();
			m_pRawPtr = TNULL;
		}
	}

	T*       Get() { return m_pRawPtr; }
	const T* Get() const { return m_pRawPtr; }

	operator T*() { return Get(); }
	operator const T*() const { return Get(); }

	T*       operator->() { return Get(); }
	const T* operator->() const { return Get(); }

	T2SharedPtr& operator=( const T2SharedPtr& a_rOther )
	{
		if ( m_pRawPtr != a_rOther.m_pRawPtr )
		{
			DebugValidate();

			if ( m_pRawPtr )
				DecrementNumRefs();

			m_pRawPtr = a_rOther.m_pRawPtr;

			if ( m_pRawPtr )
				IncrementNumRefs();
		}

		return *this;
	}

	// Use this to create shared pointer objects
	template <class... Args>
	static T2SharedPtr New( Args&&... args )
	{
		// Allocate memory
		TCHAR* pMem    = (TCHAR*)TMalloc( sizeof( T ) + sizeof( SharedPtrData_t ) );
		T*     pObject = new ( pMem + sizeof( SharedPtrData_t ) ) T( std::forward<Args>( args )... );

		// Create shared pointer object
		T2SharedPtr sharedPointer;
		sharedPointer.m_pRawPtr = pObject;

		// Set shared pointer data data
		SharedPtrData_t* pPtrData = sharedPointer.AccessPtrData();

		pPtrData->uiNumRefs = 1;
		pPtrData->uiNumWeakRefs = 0;

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
		return TREINTERPRETCAST( SharedPtrData_t*, TUINTPTR( m_pRawPtr ) - sizeof( SharedPtrData_t ) );
	}

	static SharedPtrData_t* AccessPtrData( T* a_pRawPtr )
	{
		TVALIDPTR( a_pRawPtr );
		return TREINTERPRETCAST( SharedPtrData_t*, TUINTPTR( a_pRawPtr ) - sizeof( SharedPtrData_t ) );
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

	static void DebugValidate( T* a_pRawPtr )
	{
#ifdef TOSHI2_ENABLE_SHARED_PTR_VALIDATION
		if ( a_pRawPtr )
		{
			SharedPtrData_t* pPtrData = AccessPtrData( a_pRawPtr );
			TASSERT( pPtrData->uiMagic == VALIDATION_MAGIC && pPtrData->pObjectRaw == a_pRawPtr );
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

		SharedPtrData_t* pPtrData = AccessPtrData();

		if ( --pPtrData->uiNumRefs == 0 )
		{
			// This was the only reference left, destroy the object and free memory if possible
			Get()->~T();
			m_pRawPtr = TNULL;

			if ( pPtrData->uiNumWeakRefs == 0 )
				TFree( pPtrData );
		}
	}

	static void IncrementNumWeakRefs( T* a_pRawPtr )
	{
		TASSERT( TTRUE == IsValid( a_pRawPtr ) );
		DebugValidate( a_pRawPtr );

		AccessPtrData( a_pRawPtr )->uiNumWeakRefs++;
	}

	static void DecrementNumWeakRefs( T* a_pRawPtr )
	{
		TASSERT( TTRUE == IsValid( a_pRawPtr ) );
		DebugValidate( a_pRawPtr );

		SharedPtrData_t* pPtrData = AccessPtrData( a_pRawPtr );

		if ( --pPtrData->uiNumWeakRefs == 0 )
		{
			if ( pPtrData->uiNumRefs == 0 )
			{
				TFree( pPtrData );
			}
		}
	}

private:
	T* m_pRawPtr;
};

template <typename T>
class T2WeakPtr
{
public:
	template <typename T> friend class T2SharedPtr;

public:
	// Constructor
	constexpr T2WeakPtr()
	    : m_pRawPtr( TNULL )
	{}

	// Constructor
	T2WeakPtr( const T2SharedPtr<T>& a_rOther )
	    : m_pRawPtr( a_rOther.m_pRawPtr )
	{
		if ( m_pRawPtr )
			T2SharedPtr<T>::IncrementNumWeakRefs( m_pRawPtr );
	}

	// Copy constructor
	T2WeakPtr( const T2WeakPtr& a_rOther )
	    : m_pRawPtr( a_rOther.m_pRawPtr )
	{
		if ( m_pRawPtr )
			T2SharedPtr<T>::IncrementNumWeakRefs( m_pRawPtr );
	}

	// Move constructor
	T2WeakPtr( T2WeakPtr&& a_rOther )
	    : m_pRawPtr( a_rOther.m_pRawPtr )
	{
		a_rOther.m_pRawPtr = TNULL;
	}

	// Destructor
	~T2WeakPtr()
	{
		if ( m_pRawPtr )
			T2SharedPtr<T>::DecrementNumWeakRefs( m_pRawPtr );
	}

	// Returns TTRUE if set
	TBOOL IsValid() const
	{
		return T2SharedPtr<T>::AccessPtrData( m_pRawPtr )->uiNumRefs > 0;
	}

	T* Get()
	{
		if ( m_pRawPtr )
			return ( T2SharedPtr<T>::AccessPtrData( m_pRawPtr )->uiNumRefs > 0 ) ? m_pRawPtr : TNULL;
		
		return TNULL;
	}

	const T* Get() const
	{
		if ( m_pRawPtr )
			return ( T2SharedPtr<T>::AccessPtrData( m_pRawPtr )->uiNumRefs > 0 ) ? m_pRawPtr : TNULL;

		return TNULL;
	}

	operator T*() { return Get(); }
	operator const T*() const { return Get(); }

	T*       operator->() { return Get(); }
	const T* operator->() const { return Get(); }

	void Reset()
	{
		if ( m_pRawPtr )
		{
			T2SharedPtr<T>::DecrementNumWeakRefs( m_pRawPtr );
			m_pRawPtr = TNULL;
		}
	}

	T2WeakPtr& operator=( const T2WeakPtr& a_rOther )
	{
		if ( m_pRawPtr != a_rOther.m_pRawPtr )
		{
			if ( m_pRawPtr )
				T2SharedPtr<T>::DecrementNumWeakRefs( m_pRawPtr );

			m_pRawPtr = a_rOther.m_pRawPtr;

			if ( m_pRawPtr )
				T2SharedPtr<T>::IncrementNumWeakRefs( m_pRawPtr );
		}

		return *this;
	}

	T2WeakPtr& operator=( const T2SharedPtr<T>& a_rOther )
	{
		if ( m_pRawPtr != a_rOther.m_pRawPtr )
		{
			if ( m_pRawPtr )
				T2SharedPtr<T>::DecrementNumWeakRefs( m_pRawPtr );

			m_pRawPtr = a_rOther.m_pRawPtr;

			if ( m_pRawPtr )
				T2SharedPtr<T>::IncrementNumWeakRefs( m_pRawPtr );
		}

		return *this;
	}

private:
	T* m_pRawPtr;
};

TOSHI_NAMESPACE_END
