#pragma once
#include "AUnitComponent.h"

class AInteractionEvent;

//-----------------------------------------------------------------------------
// The interaction events (see AInteractionEventManager) a unit knows about:
// the ones it has heard of and the ones it has witnessed, ten of each.
// The units get it from the AUnitGameEventData static data.
//-----------------------------------------------------------------------------
class AUnitGameEventComponent
    : public AUnitComponent
{
public:
	TDECLARE_CLASS( AUnitGameEventComponent, AUnitComponent );

	static constexpr TINT MAX_NUM_EVENTS = 10;

public:
	AUnitGameEventComponent();
	~AUnitGameEventComponent();

	//-----------------------------------------------------------------------------
	// AUnitComponent
	//-----------------------------------------------------------------------------
	virtual AUnitComponentManager* GetManager() OVERRIDE;                                // +0x10
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18

	// Remembers an event the unit has heard of. Returns TFALSE if it's known already
	// or if it didn't fit.
	TBOOL AddHeardEvent( AInteractionEvent* a_pEvent, TUINT8 a_uiUnk );

	// Remembers an event the unit has witnessed. Returns TFALSE if it's known already
	// or if it didn't fit.
	TBOOL AddWitnessedEvent( AInteractionEvent* a_pEvent, TUINT8 a_uiUnk );

	// Whether the first heard (or else the first witnessed) event has every topic of a_uiTopics (see AInteractionEvent::GetTopicFlag)
	TBOOL HasEventWithAllTopics( TUINT a_uiTopics );

	// Whether the first heard (or else the first witnessed) event has any topic of a_uiTopics
	TBOOL HasEventWithAnyTopic( TUINT a_uiTopics );

	// TRUE if the unit has heard of or witnessed the event.
	TBOOL KnowsEvent( AInteractionEvent* a_pEvent ) const;

	// Knowledge level of the event (its heard data, else its witnessed data), 0 if unknown or negative
	TINT8 GetEventKnowledge( AInteractionEvent* a_pEvent ) const;

	// TRUE if the unit has witnessed the event.
	TBOOL HasWitnessedEvent( AInteractionEvent* a_pEvent ) const;

	// Forgets the event.
	void RemoveEvent( AInteractionEvent* a_pEvent );

	// Number of used slots of the witnessed events
	TINT GetNumWitnessedEvents() const;

	AInteractionEvent* GetWitnessedEvent( TINT a_iIndex ) const { return m_apWitnessedEvents[ a_iIndex ]; }

	// The first heard (or else witnessed) event whose class is exactly a_pClass, TNULL if there's none
	AInteractionEvent* FindEventOfExactClass( Toshi::TClass* a_pClass ) const;

private:
	// The first event of a_apEvents whose class is exactly a_pClass, TNULL if there's none
	static AInteractionEvent* FindEventOfExactClass( Toshi::TClass* a_pClass, AInteractionEvent* const* a_apEvents, TINT a_iNumSlots );

	// Reads the number of events and each event's id and data
	static void LoadEvents( AHackedQuickSave::Buffer* a_pBuffer, AInteractionEvent** a_apEvents, TUINT8* a_auiData, TINT a_iNumSlots );

	// Puts a_pEvent in a free slot or, if there's none, in place of the event with the
	// highest priority value.
	static TBOOL AddEvent( AInteractionEvent* a_pEvent, AInteractionEvent** a_apEvents, TUINT8* a_auiData, TINT a_iNumSlots, TUINT8 a_uiData );

private:
	AInteractionEvent* m_apHeardEvents[ MAX_NUM_EVENTS ];     // 0x10
	AInteractionEvent* m_apWitnessedEvents[ MAX_NUM_EVENTS ]; // 0x38
	TUINT8             m_auiHeardData[ MAX_NUM_EVENTS ];      // 0x60
	TUINT8             m_auiWitnessedData[ MAX_NUM_EVENTS ];  // 0x6a
};

TSTATICASSERT( sizeof( AUnitGameEventComponent ) == 0x74 );
