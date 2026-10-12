#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TNodeList.h>
#include <Toshi/T2DList.h>
#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>

#include "AInteractionComponent.h"

class AUnit;
class AInteraction;
class AInteractionActivity;
class AInteractionActivityMessage;

//-----------------------------------------------------------------------------
// Reference to a latch, as written in the interaction scripts: "name",
// "not_name" (FLAGS_NOT) or "latch_name" (FLAGS_PERSISTENT).
// See AInteractionManager::GetLatch.
//-----------------------------------------------------------------------------
struct ALatchRef
{
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE       = 0,
		FLAGS_NOT        = BITFLAG( 0 ),
		FLAGS_PERSISTENT = BITFLAG( 1 ),
	};

	// $Barnyard: FUNCTION 0059e7b0
	ALatchRef()
	    : iLatch( 0 ), uiFlags( FLAGS_NONE )
	{}

	TINT  iLatch;  // 0x00, index into AInteractionManager's latch names
	FLAGS uiFlags; // 0x04
};

TSTATICASSERT( sizeof( ALatchRef ) == 0x8 );

using ALatchRefList = Toshi::TArray<ALatchRef>;

// Whether the list has a reference to a_iLatch with FLAGS_NOT set to a_bNot.
TBOOL ALatchRefList_Contains( const ALatchRefList& a_rcList, TINT a_iLatch, TBOOL a_bNot );

//-----------------------------------------------------------------------------
// State of the 128 latches of an interaction sequence. A latch keeps its value
// between interactions of the sequence only if it's persistent.
//-----------------------------------------------------------------------------
struct AInteractionLatches
{
	static constexpr TINT MAX_NUM_LATCHES = 128;
	static constexpr TINT NUM_WORDS       = MAX_NUM_LATCHES / 32;

	AInteractionLatches();

	// Clears every latch.
	void Reset();

	// Clears every latch that isn't persistent.
	void ClearTemporary();

	void Set( TUINT a_uiLatch, TBOOL a_bValue, TBOOL a_bPersistent );

	TBOOL IsSet( TUINT a_uiLatch ) const { return ( auiValues[ a_uiLatch >> 5 ] & ( 1 << ( a_uiLatch & 31 ) ) ) != 0; }

	// Whether every latch of the list has the value it requires.
	TBOOL Matches( const ALatchRefList& a_rcList ) const;

	// Sets every latch of the list to the value it requires.
	void Apply( const ALatchRefList& a_rcList );

	TUINT auiValues[ NUM_WORDS ];     // 0x00
	TUINT auiPersistent[ NUM_WORDS ]; // 0x10
};

TSTATICASSERT( sizeof( AInteractionLatches ) == 0x20 );

//-----------------------------------------------------------------------------
// Units that take part in an interaction, collected before a sequence is started.
//-----------------------------------------------------------------------------
struct AInteractionParticipants
{
	static constexpr TINT MAX_NUM_UNITS = 128;

	AInteractionParticipants();

	void   Add( AUnit* a_pUnit );
	AUnit* Get( TINT a_iIndex ) const;
	TINT   Find( AUnit* a_pUnit ) const;

	TINT   iNumUnits;                // 0x000
	AUnit* apUnits[ MAX_NUM_UNITS ]; // 0x004
	TUINT  uiId;                     // 0x204, unique for every instance
	TINT   iInitiator;               // 0x208, index of the unit that started the interaction, -1 if none
	TBOOL  bForced;                  // 0x20c
	TBOOL  bKeepOrder;               // 0x20d, don't let the interaction pick and sort the units

	inline static TUINT ms_uiNextId = 0; // 0x0078a8f4
};

TSTATICASSERT( sizeof( AInteractionParticipants ) == 0x210 );

//-----------------------------------------------------------------------------
// Progress of a sequence through the activities of its interaction.
//-----------------------------------------------------------------------------
struct AInteractionActivityState
{
	using FLAGS = TINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE        = 0,
		FLAGS_INITIALISED = BITFLAG( 0 ),          // set by some activities (speech, test, walk) on their first update
		FLAGS_STARTED     = TINT( BITFLAG( 31 ) ), // the sign bit, OnStart was called
	};

	AInteractionActivityState();

	void Reset();

	// Moves on to the next activity.
	void Next();

	TFLOAT fDeltaTime; // 0x00
	TFLOAT fTime;      // 0x04, time spent in the current activity
	TINT   iActivity;  // 0x08, index of the current activity
	FLAGS  iFlags;     // 0x0c
};

TSTATICASSERT( sizeof( AInteractionActivityState ) == 0x10 );

//-----------------------------------------------------------------------------
// Scratch memory for the activities of a sequence, sized for the biggest activity.
//-----------------------------------------------------------------------------
struct AInteractionActivityData
{
	AInteractionActivityData()
	    : pData( TNULL )
	{}

	~AInteractionActivityData();

	void Allocate( TSIZE a_uiSize );

	// Same as Allocate, the size is passed by reference
	void AllocateFromSize( const TINT& a_rcSize );

	void* pData; // 0x00
};

//-----------------------------------------------------------------------------
// A running interaction: the units (through their AInteractionComponent) and the
// AInteraction they currently perform. AInteractionManager owns a fixed pool of
// these and moves them between its free and active lists.
//-----------------------------------------------------------------------------
class AInteractionSequence
    : public Toshi::TObject
    , public Toshi::TNodeList<AInteractionSequence>::TNode
{
public:
	TDECLARE_CLASS( AInteractionSequence, Toshi::TObject );

	friend class AInteractionManager;

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                 = 0,
		FLAGS_PERSISTENT           = BITFLAG( 0 ), // keeps running after the interaction is over
		FLAGS_INTERACTION_DONE     = BITFLAG( 1 ), // the current interaction is over, a new one has to be picked
		FLAGS_UNK2                 = BITFLAG( 2 ), // AWalkActivity's "tocamera" stops instead
		FLAGS_ENDED                = BITFLAG( 3 ),
		FLAGS_MISSION_CONVERSATION = BITFLAG( 12 ), // AInteraction::FLAGS_MISSION_CONVERSATION of the interaction
		FLAGS_MISSION_OBJECTIVE    = BITFLAG( 13 ), // AInteraction::FLAGS_MISSION_OBJECTIVE of the interaction
	};

	// Unit categories used by requirements and activities (the "category" property).
	using CATEGORY = TINT;
	enum CATEGORY_ : CATEGORY
	{
		CATEGORY_PRIMARY    = 0,
		CATEGORY_SECONDARY  = 1,
		CATEGORY_AUXILIARY  = 2,
		CATEGORY_AUXILIARY2 = 3,
		CATEGORY_AUXILIARY3 = 4,
		CATEGORY_AUXILIARY4 = 5,
		CATEGORY_AUXILIARY5 = 6,
		CATEGORY_ACTIVE     = 7, // unit the current activity acts on
		CATEGORY_TARGET     = 8, // target of the current activity
		CATEGORY_ALL        = 9,
	};

public:
	AInteractionSequence();
	~AInteractionSequence();

	// Makes sure the activity scratch memory has at least a_iSize bytes.
	void AllocateActivityData( TINT a_iSize );

	void  RemoveComponent( AInteractionComponent* a_pComponent );
	TBOOL AddUnit( AUnit* a_pUnit );

	AInteractionActivity* GetCurrentActivity();

	// Fills a_rParticipants with the units of the sequence.
	void GetParticipants( AInteractionParticipants& a_rParticipants );

	// Makes every unit leave the sequence.
	void RemoveAllComponents( TBOOL a_bPlayIdleAnim );

	// Makes the units of the sequence ignore each other for a while.
	void IgnoreEachOther();

	// Chance (0..1) of a_pInteraction being picked for the participants, 0 if it can't be played.
	TFLOAT GetInteractionWeight( AInteraction* a_pInteraction, AInteractionParticipants* a_pParticipants );

	// Whether the sequence should end instead of picking another interaction.
	TBOOL ShouldEnd();

	AUnit* GetUnit( TINT a_iIndex );
	AUnit* GetUnitByCategory( CATEGORY a_eCategory );

	void FinishInteraction();

	// Finds a unit that has a forced interaction argument (only for sequences that just started a nameless interaction).
	TBOOL GetForcedComponent( AInteractionComponent** a_ppComponent );

	void MoveUnitToFront( AUnit* a_pUnit );

	// Ends the current interaction and makes every unit leave.
	void Reset( TBOOL a_bPlayIdleAnim );

	// Returns TTRUE when the current interaction is over.
	TBOOL Update( TFLOAT a_fDeltaTime, AInteractionComponent** a_ppForcedComponent );

	// Starts a_pInteraction with the units of a_pParticipants.
	void SetInteraction( AInteraction* a_pInteraction, AInteractionParticipants* a_pParticipants, TBOOL a_bTakeOverUnits );

	// Finds the message named a_strName (the class name if empty) the activities of the sequence share.
	AInteractionActivityMessage* FindActivityMessage( const Toshi::TClass* a_pClass, Toshi::TPString8 a_strName );

	// Finds the message named a_strName (the class name if empty), takes one of class a_pClass from the
	// manager's message groups if there is none.
	AInteractionActivityMessage* GetActivityMessage( const Toshi::TClass* a_pClass, Toshi::TPString8 a_strName );

	// Instantiations: <AKinematicsObstructionActivityMessage> 005bc8c0, <AWorldPositionActivityMessage> 005c47d0
	// $Barnyard: FUNCTION 005bc8c0
	// $Barnyard: FUNCTION 005c47d0
	template <class T>
	T* FindActivityMessage( Toshi::TPString8 a_strName )
	{
		return TSTATICCAST( T, FindActivityMessage( &TGetClass( T ), a_strName ) );
	}

	// Instantiations: <AKinematicsObstructionActivityMessage> 005ba0e0, <AWorldPositionActivityMessage> 005bd4e0
	// $Barnyard: FUNCTION 005ba0e0
	// $Barnyard: FUNCTION 005bd4e0
	template <class T>
	T* GetActivityMessage( Toshi::TPString8 a_strName )
	{
		return TSTATICCAST( T, GetActivityMessage( &TGetClass( T ), a_strName ) );
	}

	TBOOL IsEnded() const { return ( m_uiFlags & FLAGS_ENDED ) != 0; }

	// $Barnyard: FUNCTION 004300f0
	TBOOL IsInteractionOver() const { return !m_pInteraction || ( m_uiFlags & FLAGS_INTERACTION_DONE ); }
	FLAGS GetFlags() const { return m_uiFlags; }
	void  SetFlags( FLAGS a_uiFlags ) { m_uiFlags = a_uiFlags; }
	TBOOL IsStarted() const { return m_bStarted; }

	TUINT                                  GetId() const { return m_uiId; }
	AInteraction*                          GetInteraction() const { return m_pInteraction; }
	Toshi::T2DList<AInteractionComponent>& GetComponents() { return m_llComponents; }

	// $Barnyard: FUNCTION 004300b0
	Toshi::T2DList<AInteractionComponent>::Iterator GetComponentsBegin() { return m_llComponents.Begin(); }

	// $Barnyard: FUNCTION 004300d0
	Toshi::T2DList<AInteractionComponent>::Iterator GetComponentsEnd() { return m_llComponents.End(); }

	// Returns the unit of the component a_rcIt points to
	// $Barnyard: FUNCTION 0042fe20
	static AUnit*              GetOwner( const Toshi::T2DList<AInteractionComponent>::Iterator& a_rcIt ) { return a_rcIt->GetOwner(); }
	AInteractionLatches&       GetLatches() { return m_oLatches; }
	AInteractionActivityState& GetActivityState() { return m_oActivityState; }
	void*                      GetActivityData() { return m_oActivityData.pData; } // scratch memory of the current activity, see AInteractionActivity::GetDataSize

	static CATEGORY GetCategoryFromName( const TCHAR* a_szName );

	// Range of the components of the units of a category.
	Toshi::T2DList<AInteractionComponent>::Iterator GetCategoryBegin( CATEGORY a_eCategory );
	Toshi::T2DList<AInteractionComponent>::Iterator GetCategoryEnd( CATEGORY a_eCategory );

private:
	TUINT                                         m_uiId;               // 0x10, changes on every Reset, see AInteractionSequenceRef
	Toshi::T2DList<AInteractionComponent>         m_llComponents;       // 0x14
	AInteractionLatches                           m_oLatches;           // 0x1c
	AInteraction*                                 m_pInteraction;       // 0x3c
	AInteractionActivityState                     m_oActivityState;     // 0x40
	TINT                                          m_iNumInteractions;   // 0x50, interactions played since the sequence started
	FLAGS                                         m_uiFlags;            // 0x54
	Toshi::TPString8                              m_strNextInteraction; // 0x58
	AInteractionActivityData                      m_oActivityData;      // 0x5c
	Toshi::TNodeList<AInteractionActivityMessage> m_llMessages;         // 0x60, messages of the activities, see GetActivityMessage
	TBOOL                                         m_bStarted;           // 0x70

	inline static TUINT ms_uiLastId = 0; // 0x0078a928
};

TSTATICASSERT( sizeof( AInteractionSequence ) == 0x74 );

//-----------------------------------------------------------------------------
// Weak reference to a sequence: becomes TNULL once the sequence is reset.
//-----------------------------------------------------------------------------
struct AInteractionSequenceRef
{
	AInteractionSequenceRef( AInteractionSequence* a_pSequence );
	AInteractionSequenceRef( AInteractionSequenceRef& a_rOther );

	AInteractionSequenceRef& operator=( AInteractionSequence* a_pSequence );

	// Clears the reference if the sequence was reset since.
	void Validate();

	AInteractionSequence* pSequence; // 0x00
	TUINT                 uiId;      // 0x04
	TBOOL                 bSet;      // 0x08
};

TSTATICASSERT( sizeof( AInteractionSequenceRef ) == 0xc );
