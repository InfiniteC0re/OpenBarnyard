#include "ToshiPCH.h"
#include "TPString8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TDEFINE_FREELIST_ALLOCATOR1( Toshi::TPooledString8, 64 );

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006bc3e0
void TPString8Pool::Get( TPooledString8*& a_pOutString, const TCHAR* a_szString, bool* a_pWasInPool )
{
	a_pOutString = TNULL;

	if ( a_szString != TNULL )
	{
		auto iStrLen = TStringManager::String8Length( a_szString );

		if ( iStrLen != 0 )
		{
			auto it = m_oMap.FindLast( a_szString );

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

// NOTE: not in retail, initialises the unknown values like the constructor below does
TPString8Pool::TPString8Pool( T2Allocator* a_pAllocator )
    : m_pAllocator( a_pAllocator ), m_Unk1( 0 ), m_oMap( a_pAllocator ), m_Unk2( 0 ), m_Unk3( 0 ), m_Unk4( 0 ), m_Unk5( 0 )
{
}

// $Barnyard: FUNCTION 006bc170
TString8 operator+( const TString8& a_rcString, const TPString8& a_rcPString )
{
	TString8 strResult( a_rcString );
	strResult.Concat( a_rcPString.GetString8() );

	return strResult;
}

// $Barnyard: FUNCTION 006bc760
TPString8Pool::TPString8Pool( TINT a_iUnknown1, TINT a_iUnknown2, T2Allocator* a_pAllocator, void* m_pUnknown3 )
    : m_pAllocator( a_pAllocator ), m_Unk1( TUINT( m_pUnknown3 ) ), m_oMap( a_pAllocator ), m_Unk2( 0 ), m_Unk3( 0 ), m_Unk4( 0 ), m_Unk5( 0 )
{

	for ( auto it = TPString8Initialiser::Head(); it != TNULL; it = it->Next() )
	{
		it->Initialise( this );
	}
}

// $Barnyard: FUNCTION 006bc7b0
TPString8Pool::~TPString8Pool()
{
	for ( auto it = TPString8Initialiser::Head(); it != TNULL; it = it->Next() )
	{
		it->Deinitialise();
	}

	for ( auto it = m_oMap.Begin(); it != m_oMap.End(); it++ )
	{
		TPooledString8* pString = it->GetSecond();

		if ( pString )
		{
			pString->m_pPool = TNULL;
			pString->m_oString.~TString8();
			pString->m_iRefCount = -1;
			m_pAllocator->Free( pString );
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	TPString8::ms_bPoolDestroyed = TTRUE;
#endif // BARNYARD_COMMUNITY_PATCH
}

// $Barnyard: FUNCTION 006bc0a0
TPString8Initialiser::TPString8Initialiser( StringMap* a_pStrings, TUINT a_iStringCount, TBOOL a_bCanReinitialise )
{
	m_iCount            = a_iStringCount;
	m_pStrings          = a_pStrings;
	m_pNextInitialiser  = ms_pHeadInitialiser;
	m_bCanReinitialise  = a_bCanReinitialise;
	ms_pHeadInitialiser = this;
}

// $Barnyard: FUNCTION 006bc4c0
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

// $Barnyard: FUNCTION 006bc690
void TPString8Initialiser::Deinitialise()
{
	for ( TUINT i = 0; i < m_iCount; i++ )
	{
		TPString8* pString = m_pStrings[ i ].m_pString8;

		if ( TPooledString8* pPooled = pString->m_pPtr )
		{
			if ( pPooled->DecRefCount() == 1 )
				pPooled->Delete();

			pString->m_pPtr = TNULL;
		}
	}

	if ( m_bCanReinitialise )
	{
		// NOTE: the original does the same once more, the strings are already released by the loop above
		for ( TUINT i = 0; i < m_iCount; i++ )
		{
			TPString8* pString = m_pStrings[ i ].m_pString8;

			if ( TPooledString8* pPooled = pString->m_pPtr )
			{
				if ( pPooled->DecRefCount() == 1 )
					pPooled->Delete();

				pString->m_pPtr = TNULL;
			}
		}
	}
}

// $Barnyard: FUNCTION 006bc380
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
