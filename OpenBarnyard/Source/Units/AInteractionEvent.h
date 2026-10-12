#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TQList.h>
#include <Toshi/TPString8.h>

#include "GameInterface/AHackedQuickSave.h"

class AFunctionTable;
class AUnit;
struct AUnitRef;

//-----------------------------------------------------------------------------
// Line a unit can say about an event (see AInteractionEvent::GetEventText).
// NOTE: A 4 byte object returned through a hidden pointer.
//-----------------------------------------------------------------------------
struct AInteractionEventText
{
	AInteractionEventText( const TWCHAR* a_wszText = TNULL )
	    : wszText( a_wszText )
	{}

	const TWCHAR* wszText;
};

//-----------------------------------------------------------------------------
// Something that happened in the world that units can hear of or witness
// (see AUnitGameEventComponent). Events are pooled and kept in the lists of
// AInteractionEventManager, they expire once m_fDuration has passed.
//-----------------------------------------------------------------------------
class AInteractionEvent
    : public Toshi::TObject
    , public Toshi::TQList<AInteractionEvent>::TNode
{
public:
	TDECLARE_CLASS( AInteractionEvent, Toshi::TObject );

	// What the event is about (see GetTopicFlag)
	using TOPICFLAGS = TUINT16;
	enum TOPICFLAGS_ : TOPICFLAGS
	{
		TOPICFLAGS_NONE    = 0,
		TOPICFLAGS_CUSTOM  = BITFLAG( 0 ),
		TOPICFLAGS_GENERAL = BITFLAG( 1 ),
		TOPICFLAGS_UNHAPPY = BITFLAG( 2 ),
		TOPICFLAGS_HAPPY   = BITFLAG( 3 ),
		TOPICFLAGS_LOVE    = BITFLAG( 4 ),
		TOPICFLAGS_HATE    = BITFLAG( 5 ),
		TOPICFLAGS_FOOD    = BITFLAG( 6 ),
		TOPICFLAGS_FRIENDS = BITFLAG( 7 ),
		TOPICFLAGS_ENEMIES = BITFLAG( 8 ),
		TOPICFLAGS_CRIME   = BITFLAG( 9 ),
		TOPICFLAGS_MONEY   = BITFLAG( 10 ),
		TOPICFLAGS_FUN     = BITFLAG( 11 ),
		TOPICFLAGS_FEAR    = BITFLAG( 12 ),
		TOPICFLAGS_GIFT    = BITFLAG( 13 ),
		TOPICFLAGS_QUEST   = BITFLAG( 14 ),
		TOPICFLAGS_PLAYER  = BITFLAG( 15 ),
	};

public:
	AInteractionEvent();
	~AInteractionEvent();

	virtual AFunctionTable*       GetFunctionTable();                                                // +0x0c
	virtual void                  Save( AHackedQuickSave::Buffer* a_pBuffer );                       // +0x10
	virtual void                  Load( AHackedQuickSave::Buffer* a_pBuffer );                       // +0x14
	virtual AInteractionEventText GetEventText( AUnit* a_pUnit ) { return AInteractionEventText(); } // +0x18, DOES_NOTHING (0054df00) that leaves the result unconstructed
	virtual TBOOL                 Initialise( TINT a_iUnk ) { return TTRUE; }                        // +0x1c, RETURNS_TRUE (00413920), called with 0 once the event is created
	virtual TBOOL                 Update( TFLOAT a_fDeltaTime );                                     // +0x20, TFALSE once the event expired
	// $Barnyard: FUNCTION 005c5120
	virtual void       SetId( TUINT a_uiId ) { m_uiId = a_uiId; }     // +0x24, folded setter 004ce110 (005c5120 jumps to it)
	virtual void       Reset();                                       // +0x28, makes every unit forget the event
	virtual TBOOL      CanBeFreed();                                  // +0x2c, forgets the event unless a unit witnessed it
	virtual void       SetUnitRef( AUnitRef* a_pUnitRef ) {}          // +0x30, DOES_NOTHING (00628f40), the unit the event is about
	virtual void       SetUnit( AUnit* a_pUnit ) {}                   // +0x34, DOES_NOTHING (00628f40), the unit the event is about
	virtual TOPICFLAGS GetTopicFlags( void* a_pUnk );                 // +0x38, topic flags (see GetTopicFlag)
	virtual TFLOAT     GetGlobalKnowledgeOfEvent() { return 0.0f; }   // +0x3c, shared 0056ce20
	virtual void       Notify( AUnit* a_pUnit, TUINT8 a_uiUnk );      // +0x40, lets the unit know about the event
	virtual TBOOL      IsWitness( AUnit* a_pUnit ) { return TFALSE; } // +0x44, RETURNS_FALSE (006ce000)
	virtual TBOOL      IsPersistent() { return TFALSE; }              // +0x48, RETURNS_FALSE (00660960), expired events are kept if TRUE
	virtual TBOOL      GetMaxKnowledgeLevel() { return TFALSE; }      // +0x4c, RETURNS_FALSE (00660960)

	// Share of the living units that know about the event (0 to 1)
	TFLOAT ComputeGlobalKnowledge();

	// Flag (1 << index) of the topic named a_rcName ("custom", "general", "unhappy", ...), 0 if there's no such topic
	static TUINT GetTopicFlag( const Toshi::TPString8& a_rcName );

	// Whether the unit knows about the event (see AUnitGameEventComponent)
	TBOOL IsKnownBy( AUnit* a_pUnit );

	TUINT8 GetPriority() const { return m_uiPriority; }

	// How interesting the event still is: m_fInterest fading out over m_fDuration (m_uiFadeCurve picks the curve)
	TFLOAT GetInterest() const;

	TUINT  GetId() const { return m_uiId; }
	TFLOAT GetTime() const { return m_fTime; }

public:
	static AFunctionTable ms_oFunctionTable;

protected:
	TOPICFLAGS m_uiTopicFlags; // 0x0c
	TUINT8     m_uiPriority;   // 0x0e, the events with the highest value are forgotten first
	TUINT8     m_uiFadeCurve;  // 0x0f
	TFLOAT     m_fDuration;    // 0x10
	TFLOAT     m_fInterest;    // 0x14
	TFLOAT     m_fTime;        // 0x18
	TUINT      m_uiId;         // 0x1c, see AInteractionEventManager::ActivateEvent
};

TSTATICASSERT( sizeof( AInteractionEvent ) == 0x20 );
