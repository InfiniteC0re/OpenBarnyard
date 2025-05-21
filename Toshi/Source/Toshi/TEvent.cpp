#include "ToshiPCH.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c4630
void TGenericListener::Connect( TGenericEmitter& a_rEmitter, void* a_pCaller, t_Callback a_pCallback, TINT a_iPriority )
{
	TASSERT( IsLinked() == TFALSE );
	m_pCaller   = a_pCaller;
	m_pCallback = a_pCallback;
	SetPriority( a_iPriority );
	a_rEmitter.GetListeners().Insert( this );
}

// $Barnyard: FUNCTION 006c46c0
void TGenericListener::Disconnect()
{
	Remove();
	m_pCaller   = TNULL;
	m_pCallback = TNULL;
}

TGenericListener::~TGenericListener()
{
	Disconnect();
}

void TGenericListener::Execute( void* pOwner, void* pData )
{
	m_pCallback( m_pCaller, pOwner, pData );
}

// $Barnyard: FUNCTION 006c4680
void TGenericEmitter::Destroy()
{
	if ( !m_Listeners.IsEmpty() )
	{
		for ( auto it = m_Listeners.Begin(); it != m_Listeners.End(); it++ )
		{
			it->Disconnect();
		}
	}

	m_pOwner = TNULL;
}

void TGenericEmitter::Create( void* a_pOwner )
{
	m_pOwner = a_pOwner;
}

// $Barnyard: FUNCTION 006c45e0
void TGenericEmitter::Throw( void* a_pData )
{
	auto pListener = m_Listeners.Begin();

	while ( pListener != m_Listeners.End() )
	{
		auto pNextListener = pListener->Next();
		pListener->Execute( m_pOwner, a_pData );
		pListener = pNextListener;
	}
}

TGenericEmitter::TGenericEmitter( void* a_pOwner )
{
	Create( a_pOwner );
}

TGenericEmitter::TGenericEmitter()
{
	Create( TNULL );
}

TOSHI_NAMESPACE_END
