#pragma once

namespace Toshi
{

class TRefCounted
{
public:
    TRefCounted() { m_iRefCount = 0; }
    TRefCounted( TRefCounted const& ) { m_iRefCount = 0; }
    ~TRefCounted() { m_iRefCount = -1; }

    TINT         DecRefCount() { return m_iRefCount--; }
    TINT         GetRefCount() { return m_iRefCount; }
    TINT         IncRefCount() { return m_iRefCount++; }
    TRefCounted& operator=( TRefCounted const& ) { return *this; }

protected:
    TINT m_iRefCount;
};

/**
	 * T should have Delete method
	 */
template < class T >
class TRef
{
public:
    TRef() { Create( TNULL ); }
    TRef( T* a_pPtr ) { Create( a_pPtr ); }
    TRef( const TRef& a_rOther ) { Create( a_rOther.m_pPtr ); }
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
} // namespace Toshi
