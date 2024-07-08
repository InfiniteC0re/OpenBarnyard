#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: this type of pointer is the only managed pointer that has rights
// to destroy the created object.
// Note: previously known as TManagedPtr.
//-----------------------------------------------------------------------------
template <typename T>
class T2ManagedPtr
{
public:
	// Constructor
	T2ManagedPtr() :
		m_pRawPtr( TNULL )
	{ }

	// Copy constructor
	T2ManagedPtr( const T2ManagedPtr& a_rOther ) = delete;

	// Move constructor
	T2ManagedPtr( T2ManagedPtr&& a_rOther )
	{
		m_pRawPtr = a_rOther.m_pRawPtr;
		a_rOther.m_pRawPtr = TNULL;
	}

	// Destructor
	~T2ManagedPtr()
	{
		if ( m_pRawPtr )
			delete m_pRawPtr;
	}

	// Returns TTRUE if not TNULL
	TBOOL IsValid() const
	{
		return m_pRawPtr;
	}

	// Returns TTRUE if destroyed the object
	TBOOL Destroy()
	{
		if ( m_pRawPtr )
		{
			delete m_pRawPtr;
			m_pRawPtr = TNULL;

			return TTRUE;
		}

		return TFALSE;
	}

	T* Get() { return TREINTERPRETCAST( T*, m_pRawPtr ); }
	const T* Get() const { return TREINTERPRETCAST( const T*, m_pRawPtr ); }

	T* operator->() { return Get(); }
	const T* operator->() const { return Get(); }

	T2ManagedPtr& operator=( const T2ManagedPtr& a_rOther ) = delete;

	// Use this to create managed pointer objects
	template<class... Args>
	static T2ManagedPtr New( Args&& ...args )
	{
		// Allocate memory
		T* pObject = new T( std::forward<Args>( args )... );

		return T2ManagedPtr { .m_pRawPtr = pObject };
	}

private:
	T* m_pRawPtr;
};

TOSHI_NAMESPACE_END
