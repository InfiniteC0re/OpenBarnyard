#pragma once
#include "Toshi/Toshi.h"

TOSHI_NAMESPACE_START

class TRefCounted
{
public:
	TRefCounted() { m_iRefCount = 0; }
	~TRefCounted() { m_iRefCount = -1; }

	TINT         DecRefCount() { return m_iRefCount--; }
	TINT         GetRefCount() { return m_iRefCount; }
	TINT         IncRefCount() { return m_iRefCount++; }
	TRefCounted& operator=( TRefCounted const& ) { return *this; }

protected:
	TINT m_iRefCount;
};

// typename T should have Delete method in order to be successfully destroyed
// NOTE: Each TObject has it by default
template <class T>
class TRef
{
public:
	// constructors/destructor
	constexpr TRef()
	    : m_pPtr( TNULL )
	{
	}

	TRef( T* a_pPtr )
	    : m_pPtr( TNULL )
	{
		Create( a_pPtr );
	}

	TRef( const TRef& a_rcOther )
	    : m_pPtr( TNULL )
	{
		Create( a_rcOther.m_pPtr );
	}

	TRef( TRef&& a_rOther )
	{
		a_rOther.m_pPtr = m_pPtr;
		m_pPtr          = TNULL;
	}

	~TRef() { Destroy(); }

	TRef& operator=( T* a_pPtr )
	{
		if ( a_pPtr != m_pPtr )
		{
			Create( a_pPtr );
		}

		return *this;
	}

	TRef& operator=( const TRef& a_rOther )
	{
		if ( a_rOther.m_pPtr != m_pPtr )
		{
			Create( a_rOther.m_pPtr );
		}

		return *this;
	}

	T*       Get() { return m_pPtr; }
	const T* Get() const { return m_pPtr; }

	T*       operator->() { return m_pPtr; }
	const T* operator->() const { return m_pPtr; }

	operator T*() { return m_pPtr; }
	operator const T*() const { return m_pPtr; }

private:
	TFORCEINLINE void Create( T* a_pPtr )
	{
		Destroy();
		m_pPtr = a_pPtr;

		if ( m_pPtr )
		{
			m_pPtr->IncRefCount();
		}
	}

	TFORCEINLINE void Destroy()
	{
		if ( m_pPtr && m_pPtr->DecRefCount() == 1 )
		{
			m_pPtr->Delete();
		}

		m_pPtr = TNULL;
	}

protected:
	T* m_pPtr;
};

TOSHI_NAMESPACE_END
