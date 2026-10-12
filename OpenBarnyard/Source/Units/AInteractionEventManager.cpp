#include "pch.h"
#include "AInteractionEventManager.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AFunctionPointer.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005c5b70, CreateObject 005c6ad0, CreateObjectInPlace 005c6b70
TDEFINE_CLASS_INIT( AInteractionEventManager );

AFunctionTable AInteractionEventManager::ms_oFunctionTable( &TGetClass( AInteractionEventManager ), TNULL );

// FSM fallback: AInteractionEventManager natives always run on the singleton.
// $Barnyard: FUNCTION 005c5c00
static TObject* __stdcall AInteractionEventManagerFunctionFallback( void* a_pUnk1, void* a_pUnk2 )
{
	return AInteractionEventManager::GetSingleton();
}

// FSM "GetInteractionEventManagerNonInline".
// $Barnyard: FUNCTION 005c5c10
static AInteractionEventManager* GetInteractionEventManagerNonInline()
{
	return AInteractionEventManager::GetSingleton();
}

// $Barnyard: FUNCTION 005c6960
void AInteractionEventManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AInteractionEventManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AFunctionPointer0ArgsRT<AInteractionEventManager*>::Create( "GetInteractionEventManagerNonInline", &GetInteractionEventManagerNonInline ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<AInteractionEvent*, AInteractionEventManager, TPString8>::Create( "FindEvent", &AInteractionEventManager::FindEventByClassName ) );
}

// $Barnyard: FUNCTION 005c5b80
void AInteractionEventManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// NOTE: inlined into CreateObject 005c6ad0 / CreateObjectInPlace 005c6b70
AInteractionEventManager::AInteractionEventManager()
    : m_iNumActiveEvents( 0 )
    , m_iMaxNumEvents( DEFAULT_MAX_NUM_EVENTS )
    , m_pEventPool( TNULL )
{
}

// $Barnyard: FUNCTION 005c6c10
// $Barnyard: FUNCTION 005c69c0
AInteractionEventManager::~AInteractionEventManager()
{
	Reset();

	if ( m_pEventPool )
	{
		delete m_pEventPool;
		m_pEventPool = TNULL;
	}
}

// $Barnyard: FUNCTION 005c5ff0
TBOOL AInteractionEventManager::Create()
{
	m_iNumActiveEvents = 0;
	m_pEventPool       = new T2DerivedDynamicObjectPool<AInteractionEvent>( GetGlobalAllocator(), m_iMaxNumEvents );

	return TTask::Create();
}

// $Barnyard: FUNCTION 005c67a0
TBOOL AInteractionEventManager::Reset()
{
	while ( !m_llActive.IsEmpty() )
		m_pEventPool->DeleteObject( m_llActive.Pop() );

	while ( !m_llFree.IsEmpty() )
		m_pEventPool->DeleteObject( m_llFree.Pop() );

	while ( !m_llExpired.IsEmpty() )
		m_pEventPool->DeleteObject( m_llExpired.Pop() );

	m_iNumActiveEvents = 0;
	return TTRUE;
}

// $Barnyard: FUNCTION 005c5ef0
TBOOL AInteractionEventManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	for ( AInteractionEvent* pEvent = m_llActive.Begin(); pEvent != m_llActive.End(); )
	{
		AInteractionEvent* pNext = pEvent->Next();

		if ( !pEvent->Update( a_fDeltaTime ) )
		{
			// The event expired
			TBOOL bCanBeFreed = pEvent->CanBeFreed();
			TBOOL bPersistent = pEvent->IsPersistent();

			m_llActive.Remove( pEvent );

			if ( !bPersistent && bCanBeFreed )
			{
				m_iNumActiveEvents--;
				m_llFree.Push( pEvent );
			}
			else
			{
				m_llExpired.Push( pEvent );
			}
		}

		pEvent = pNext;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005c69b0
AFunctionTable* AInteractionEventManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 005c5b90
void AInteractionEventManager::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "InteractionEventManager" );
	a_pBuffer->WriteInt( TINT( ms_uiNextEventId ) );
	a_pBuffer->WriteInt( m_iNumActiveEvents );
	SaveEvents( a_pBuffer, &m_llActive );
	SaveEvents( a_pBuffer, &m_llExpired );
}

// $Barnyard: FUNCTION 005c6440
void AInteractionEventManager::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "InteractionEventManager" );

	a_pBuffer->ReadUInt( ms_uiNextEventId );
	a_pBuffer->ReadInt( m_iNumActiveEvents );
	LoadEvents( a_pBuffer, &m_llActive );
	LoadEvents( a_pBuffer, &m_llExpired );

	// NOTE: retail walks both lists here without doing anything (stripped debug code)
}

// $Barnyard: FUNCTION 005c6500
void AInteractionEventManager::SaveEvents( AHackedQuickSave::Buffer* a_pBuffer, EventList* a_pList )
{
	TINT iNumEvents = 0;

	for ( AInteractionEvent* pEvent = a_pList->Begin(); pEvent != a_pList->End(); pEvent = pEvent->Next() )
		iNumEvents++;

	a_pBuffer->WriteInt( iNumEvents );

	for ( AInteractionEvent* pEvent = a_pList->Begin(); pEvent != a_pList->End(); pEvent = pEvent->Next() )
	{
		a_pBuffer->WriteString( pEvent->GetClass()->GetName() );
		pEvent->Save( a_pBuffer );
	}
}

// $Barnyard: FUNCTION 005c65b0
void AInteractionEventManager::LoadEvents( AHackedQuickSave::Buffer* a_pBuffer, EventList* a_pList )
{
	// NOTE: static buffer at 0x007ce238 in retail
	static TCHAR s_szClassName[ 512 ];

	TINT iNumEvents = 0;
	a_pBuffer->ReadInt( iNumEvents );

	for ( TINT i = 0; i < iNumEvents; i++ )
	{
		a_pBuffer->ReadString( s_szClassName );

		const TClass* pClass = TClass::Find( s_szClassName, TNULL );
		// NOTE: retail asks the pool for GetClassSize() bytes, the pool ignores the size anyway
		AInteractionEvent* pEvent = TSTATICCAST( AInteractionEvent, pClass->CreateObjectInPlace( m_pEventPool->AllocateObject() ) );

		pEvent->Initialise( 0 );
		pEvent->Reset();

		// NOTE: retail adds the event to the free ones first
		m_llFree.Push( pEvent );
		pEvent->Remove();
		a_pList->PushFront( pEvent );

		pEvent->Load( a_pBuffer );
	}
}

// $Barnyard: FUNCTION 005c63e0
AInteractionEvent* AInteractionEventManager::NewEvent( const TClass* a_pClass )
{
	// NOTE: retail asks the pool for GetClassSize() bytes, the pool ignores the size anyway
	AInteractionEvent* pEvent = TSTATICCAST( AInteractionEvent, a_pClass->CreateObjectInPlace( m_pEventPool->AllocateObject() ) );

	pEvent->Initialise( 0 );
	pEvent->Reset();
	m_llFree.Push( pEvent );

	return pEvent;
}

// $Barnyard: FUNCTION 005c6310
AInteractionEvent* AInteractionEventManager::ActivateEvent( const TClass* a_pClass )
{
	for ( AInteractionEvent* pEvent = m_llFree.Begin(); pEvent != m_llFree.End(); pEvent = pEvent->Next() )
	{
		if ( pEvent->GetClass()->IsA( a_pClass ) )
		{
			pEvent->Reset();

			// Zero is not a valid id
			TUINT uiId = ms_uiNextEventId++;

			if ( ms_uiNextEventId == 0 )
				ms_uiNextEventId = 1;

			pEvent->SetId( uiId );

			m_llFree.Remove( pEvent );
			m_llActive.PushFront( pEvent );
			m_iNumActiveEvents++;

			return pEvent;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005c6780
AInteractionEvent* AInteractionEventManager::NewActiveEvent( const TClass* a_pClass )
{
	NewEvent( a_pClass );
	return ActivateEvent( a_pClass );
}

// $Barnyard: FUNCTION 005c6070
AInteractionEvent* AInteractionEventManager::FindEvent( const TClass* a_pClass )
{
	for ( AInteractionEvent* pEvent = m_llActive.Begin(); pEvent != m_llActive.End(); pEvent = pEvent->Next() )
	{
		if ( pEvent->GetClass()->IsA( a_pClass ) )
			return pEvent;
	}

	for ( AInteractionEvent* pEvent = m_llExpired.Begin(); pEvent != m_llExpired.End(); pEvent = pEvent->Next() )
	{
		if ( pEvent->GetClass()->IsA( a_pClass ) )
			return pEvent;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005c66b0
AInteractionEvent* AInteractionEventManager::FindEventByClassName( TPString8 a_strClassName )
{
	const TClass* pClass = TClass::Find( a_strClassName.GetString(), TNULL );

	Iterator oIterator;
	oIterator.pEvent = m_llActive.Begin();
	oIterator.pList  = &m_llActive;

	AInteractionEvent* pEvent = FindInList( &m_llActive, pClass, oIterator );

	if ( !pEvent )
	{
		oIterator.pEvent = m_llExpired.Begin();
		oIterator.pList  = &m_llExpired;

		pEvent = FindInList( &m_llExpired, pClass, oIterator );
	}

	return pEvent;
}

// $Barnyard: FUNCTION 005c6270
AInteractionEvent* AInteractionEventManager::FindEventById( TUINT a_uiId )
{
	for ( AInteractionEvent* pEvent = m_llActive.Begin(); pEvent != m_llActive.End(); pEvent = pEvent->Next() )
	{
		if ( a_uiId == pEvent->GetId() )
			return pEvent;
	}

	for ( AInteractionEvent* pEvent = m_llExpired.Begin(); pEvent != m_llExpired.End(); pEvent = pEvent->Next() )
	{
		if ( a_uiId == pEvent->GetId() )
			return pEvent;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005c61b0
AInteractionEvent* AInteractionEventManager::FindFirstEvent( const TClass* a_pClass, Iterator& a_rIterator )
{
	a_rIterator.pEvent = m_llActive.Begin();
	a_rIterator.pList  = &m_llActive;

	AInteractionEvent* pEvent = FindInList( &m_llActive, a_pClass, a_rIterator );

	if ( pEvent )
		return pEvent;

	a_rIterator.pEvent = m_llExpired.Begin();
	a_rIterator.pList  = &m_llExpired;

	return FindInList( &m_llExpired, a_pClass, a_rIterator );
}

// $Barnyard: FUNCTION 005c6210
AInteractionEvent* AInteractionEventManager::FindNextEvent( const TClass* a_pClass, Iterator& a_rIterator )
{
	if ( a_rIterator.pList == &m_llActive )
	{
		AInteractionEvent* pEvent = FindInList( &m_llActive, a_pClass, a_rIterator );

		if ( pEvent )
			return pEvent;
	}

	// Continue with the expired events
	if ( a_rIterator.pList != &m_llExpired )
	{
		a_rIterator.pEvent = m_llExpired.Begin();
		a_rIterator.pList  = &m_llExpired;
	}

	return FindInList( &m_llExpired, a_pClass, a_rIterator );
}

// $Barnyard: FUNCTION 005c6130
AInteractionEvent* AInteractionEventManager::FindInList( EventList* a_pList, const TClass* a_pClass, Iterator& a_rIterator )
{
	while ( a_rIterator.pEvent != a_pList->End() )
	{
		AInteractionEvent* pEvent = a_rIterator.pEvent;
		a_rIterator.pEvent        = pEvent->Next();

		if ( pEvent->GetClass()->IsA( a_pClass ) )
			return pEvent;
	}

	return TNULL;
}
