#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TQList.h>
#include <Toshi/T2ObjectPool.h>

#include "AInteractionEvent.h"
#include "GameInterface/AHackedQuickSave.h"

class AFunctionTable;

//-----------------------------------------------------------------------------
// Owns the interaction events: a pool they are created in, the free ones, the
// active ones (updated every frame) and the expired ones that still matter
// (witnessed or persistent). Created by AInteractionManager::Create.
//-----------------------------------------------------------------------------
class AInteractionEventManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AInteractionEventManager>
{
public:
	TDECLARE_CLASS( AInteractionEventManager, Toshi::TTask );

	static constexpr TINT DEFAULT_MAX_NUM_EVENTS = 200;

	using EventList = Toshi::TQList<AInteractionEvent>;

	// Position of a search over the active and the expired events
	struct Iterator
	{
		AInteractionEvent* pEvent; // 0x00, next event to check
		EventList*         pList;  // 0x04
	};

public:
	AInteractionEventManager();
	~AInteractionEventManager();

	virtual TBOOL Create() OVERRIDE;                        // +0x0c
	virtual TBOOL Reset() OVERRIDE;                         // +0x14
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x1c

	virtual AFunctionTable* GetFunctionTable();                                                    // +0x38
	virtual void            Save( AHackedQuickSave::Buffer* a_pBuffer );                           // +0x3c
	virtual void            Load( AHackedQuickSave::Buffer* a_pBuffer );                           // +0x40
	virtual void            SaveEvents( AHackedQuickSave::Buffer* a_pBuffer, EventList* a_pList ); // +0x44
	virtual void            LoadEvents( AHackedQuickSave::Buffer* a_pBuffer, EventList* a_pList ); // +0x48

	// Creates an event of the class and adds it to the free events.
	// NOTE: not called CreateEvent because of the Windows macro.
	AInteractionEvent* NewEvent( const Toshi::TClass* a_pClass );

	// Takes a free event of the class, gives it a new id and makes it active.
	AInteractionEvent* ActivateEvent( const Toshi::TClass* a_pClass );

	// NewEvent + ActivateEvent.
	AInteractionEvent* NewActiveEvent( const Toshi::TClass* a_pClass );

	// First active or expired event of the class.
	AInteractionEvent* FindEvent( const Toshi::TClass* a_pClass );

	// FSM version of FindEvent.
	AInteractionEvent* FindEventByClassName( Toshi::TPString8 a_strClassName );

	// Active or expired event with the id.
	AInteractionEvent* FindEventById( TUINT a_uiId );

	// Starts a search over the active and the expired events of the class, see FindNextEvent.
	AInteractionEvent* FindFirstEvent( const Toshi::TClass* a_pClass, Iterator& a_rIterator );

	// Continues the search started by FindFirstEvent.
	AInteractionEvent* FindNextEvent( const Toshi::TClass* a_pClass, Iterator& a_rIterator );

private:
	// Returns the event a_rIterator points to (or the next one) if it's of the class and moves the iterator past it.
	AInteractionEvent* FindInList( EventList* a_pList, const Toshi::TClass* a_pClass, Iterator& a_rIterator );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	EventList                                             m_llActive;         // 0x20
	EventList                                             m_llFree;           // 0x28
	EventList                                             m_llExpired;        // 0x30
	TINT                                                  m_iNumActiveEvents; // 0x38
	TINT                                                  m_iMaxNumEvents;    // 0x3c
	Toshi::T2DerivedDynamicObjectPool<AInteractionEvent>* m_pEventPool;       // 0x40

	inline static TUINT ms_uiNextEventId = 1; // 0x00775398
};

TSTATICASSERT( sizeof( AInteractionEventManager ) == 0x44 );
