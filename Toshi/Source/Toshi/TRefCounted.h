#pragma once
#include "Toshi/Toshi.h"

TOSHI_NAMESPACE_START

class MODAPI TRefCounted
{
public:
	TRefCounted() { m_iRefCount = 0; }
	// $Barnyard: FUNCTION 0040d7e0
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
	// NOTE: every instantiation is folded into one out of line copy, used by the vector constructor iterators: 00440e20
	// $Barnyard: FUNCTION 00440e20
	constexpr TRef()
	    : m_pPtr( TNULL )
	{
	}

	TRef( T* a_pPtr )
	    : m_pPtr( TNULL )
	{
		Create( a_pPtr );
	}

	// TRef<AUnitProxy> 0040d8a0
	// $Barnyard: FUNCTION 0040d8a0
	TRef( const TRef& a_rcOther )
	    : m_pPtr( TNULL )
	{
		Create( a_rcOther.m_pPtr );
	}

	TRef( TRef&& a_rOther )
	{
		m_pPtr          = a_rOther.m_pPtr;
		a_rOther.m_pPtr = TNULL;
	}

	// NOTE: 006b1c50 is the scalar deleting destructor of TRef<PPropertyValueArray>
	// $Barnyard: FUNCTION 006b1c50
	// Folded destructor of the TRef instances (AUnitProxy, ...) 005c5330
	// $Barnyard: FUNCTION 005c5330
	~TRef() { Destroy(); }

	// NOTE: TRef<PProperties> (006b1b80), TRef<PPropertyValueArray> (006b1bb0)
	// $Barnyard: FUNCTION 006b1b80
	// $Barnyard: FUNCTION 006b1bb0
	TRef& operator=( T* a_pPtr )
	{
		if ( a_pPtr != m_pPtr )
		{
			Create( a_pPtr );
		}

		return *this;
	}

	// TRef<AUnitProxy> 0040d8c0
	// $Barnyard: FUNCTION 0040d8c0
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
