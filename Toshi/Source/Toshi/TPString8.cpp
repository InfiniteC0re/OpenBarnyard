#include "ToshiPCH.h"
#include "TPString8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TDEFINE_FREELIST_ALLOCATOR1( Toshi::TPooledString8, 64 );

TOSHI_NAMESPACE_START

void TPString8Pool::Get( TPooledString8*& a_pOutString, const TCHAR* a_szString, bool* a_pWasInPool )
{
	a_pOutString = TNULL;

	if ( a_szString != TNULL )
	{
		auto iStrLen = TStringManager::String8Length( a_szString );

		if ( iStrLen != 0 )
		{
			auto it = m_oMap.Find( a_szString );

			if ( m_oMap.IsValid( it ) )
			{
				if ( a_pWasInPool )
					*a_pWasInPool = TTRUE;

				a_pOutString = it->GetSecond();
				a_pOutString->IncRefCount();
			}
			else
			{
				TPooledString8* pString = m_pAllocator->New<TPooledString8>( a_szString, this, m_pAllocator );
				m_oMap.Insert( pString->GetString8().GetString(), pString );

				a_pOutString = pString;
				pString->IncRefCount();
			}
		}
	}
}

TPString8Pool::TPString8Pool( T2Allocator* a_pAllocator )
    : m_pAllocator( a_pAllocator ), m_oMap( a_pAllocator )
{
	// TODO: Initialise some unknown values
}

TPString8Pool::TPString8Pool( TINT a_iUnknown1, TINT a_iUnknown2, T2Allocator* a_pAllocator, void* m_pUnknown3 )
    : m_pAllocator( a_pAllocator ), m_oMap( a_pAllocator )
{
	// TODO: Initialise some unknown values

	for ( auto it = TPString8Initialiser::Head(); it != TNULL; it = it->Next() )
	{
		it->Initialise( this );
	}
}

TPString8Initialiser::TPString8Initialiser( StringMap* a_pStrings, TUINT a_iStringCount, TBOOL a_bCanReinitialise )
{
	m_iCount            = a_iStringCount;
	m_pStrings          = a_pStrings;
	m_pNextInitialiser  = ms_pHeadInitialiser;
	m_bCanReinitialise  = a_bCanReinitialise;
	ms_pHeadInitialiser = this;
}

void TPString8Initialiser::Initialise( TPString8Pool* a_pStringPool )
{
	if ( m_bCanReinitialise )
	{
		for ( TSIZE i = 0; i < m_iCount; i++ )
		{
			if ( m_pStrings[ i ].m_pString8 )
			{
				m_pStrings[ i ].m_pString8->SetPooledString( TNULL );
			}
		}
	}

	for ( TSIZE i = 0; i < m_iCount; i++ )
	{
		TPString8 string( a_pStringPool, m_pStrings[ i ].m_szCString );

		if ( string.GetPooledString() &&
		     string.GetPooledString() != m_pStrings[ i ].m_pString8->GetPooledString() )
		{
			*m_pStrings[ i ].m_pString8 = string;
		}
	}
}

void TPooledString8::Delete()
{
	if ( IsPooled() )
	{
		m_pPool->Remove( this );
		m_pPool->GetAllocator()->Delete( this );
	}
	else
	{
		delete this;
	}
}

TOSHI_NAMESPACE_END
