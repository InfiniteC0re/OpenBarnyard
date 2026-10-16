#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TArray.h>
#include <Toshi/TNodeList.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Map.h>
#include <Toshi/TGlobalEmitter.h>
#include <ToshiTools/T2DynamicVector.h>
#include <Math/TRandom.h>
#include <Plugins/PPropertyParser/PBProperties.h>

#include "AInteractionSequence.h"
#include "GameInterface/AHackedQuickSave.h"

class AUnit;
class AFunctionTable;
class AInteraction;
class AInteractionActivity;
class AInteractionRequirement;
class AInteractionActivityMessageGroups;
class AInteractionEventManager;

//-----------------------------------------------------------------------------
// Thrown through TGlobalEmitter when interaction sequences start and end and
// once the interactions are loaded. Listener list lives at 0x0077dd78.
//-----------------------------------------------------------------------------
class AInteractionManagerEvent
    : public Toshi::TGlobalEvent<AInteractionManagerEvent>
{
public:
	// The listeners test the bits
	using TYPE = TUINT8;
	enum TYPE_ : TYPE
	{
		TYPE_NONE                = 0,
		TYPE_UNK0                = BITFLAG( 0 ), // set with TYPE_STARTED
		TYPE_STARTED             = BITFLAG( 1 ),
		TYPE_ENDED               = BITFLAG( 2 ),
		TYPE_SEQUENCE            = BITFLAG( 3 ),
		TYPE_INTERACTIONS_LOADED = BITFLAG( 6 ),
		TYPE_SEQUENCE_STARTED    = TYPE_SEQUENCE | TYPE_STARTED | TYPE_UNK0, // 0x0b
		TYPE_SEQUENCE_ENDED      = TYPE_SEQUENCE | TYPE_ENDED,               // 0x0c
	};

	// What kind of sequence the event is about (see GetSequenceKind)
	using KIND = TINT;
	enum KIND_ : KIND
	{
		KIND_NORMAL               = 0xb, // also used by TYPE_INTERACTIONS_LOADED
		KIND_MISSION_CONVERSATION = 0xc, // AInteractionSequence::FLAGS_MISSION_CONVERSATION
		KIND_MISSION_OBJECTIVE    = 0xd, // AInteractionSequence::FLAGS_MISSION_OBJECTIVE
	};

public:
	// NOTE: retail initialises the type with 1 in some places and overwrites it right after
	AInteractionManagerEvent( AInteractionSequence* a_pSequence, TYPE a_eType, KIND a_iKind )
	    : m_pSequence( a_pSequence )
	    , m_iUnk8( 0 )
	    , m_iUnkC( 0 )
	    , m_eType( a_eType )
	    , m_iKind( a_iKind )
	{}

	// Kind of a sequence event, depends on FLAGS_MISSION_CONVERSATION / FLAGS_MISSION_OBJECTIVE of the interaction.
	static KIND GetSequenceKind( AInteractionSequence::FLAGS a_uiFlags )
	{
		if ( a_uiFlags & AInteractionSequence::FLAGS_MISSION_CONVERSATION ) return KIND_MISSION_CONVERSATION;
		if ( a_uiFlags & AInteractionSequence::FLAGS_MISSION_OBJECTIVE ) return KIND_MISSION_OBJECTIVE;
		return KIND_NORMAL;
	}

	AInteractionSequence* GetSequence() const { return m_pSequence; }
	TYPE                  GetType() const { return m_eType; }
	KIND                  GetKind() const { return m_iKind; }

private:
	AInteractionSequence* m_pSequence; // 0x04
	TINT                  m_iUnk8;     // 0x08
	TINT                  m_iUnkC;     // 0x0c
	TYPE                  m_eType;     // 0x10
	KIND                  m_iKind;     // 0x14
};

//-----------------------------------------------------------------------------
// Loads the interactions (Data/Interactions/*.trz) with their requirements and
// activities and runs the interaction sequences that units take part in.
// Units ask for interactions through their AInteractionComponent.
//-----------------------------------------------------------------------------
class AInteractionManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AInteractionManager>
{
public:
	TDECLARE_CLASS( AInteractionManager, Toshi::TTask );

	friend AInteractionSequence;
	friend class AInteraction;

	static constexpr TINT MAX_NUM_SEQUENCES          = 32;
	static constexpr TINT MAX_NUM_PERSISTENT_LATCHES = 128;
	static constexpr TINT MAX_NUM_CANDIDATES         = 200;

	// Flags of AssignInteractionImpl
	using ASSIGNFLAGS = TUINT;
	enum ASSIGNFLAGS_ : ASSIGNFLAGS
	{
		ASSIGNFLAGS_NONE               = 0,
		ASSIGNFLAGS_FIRST_IS_INITIATOR = BITFLAG( 0 ), // AssignInteractionToUnits: the first unit started the interaction
		ASSIGNFLAGS_PERSISTENT         = BITFLAG( 1 ), // AssignInteractionToUnits: AInteractionSequence::FLAGS_PERSISTENT
		ASSIGNFLAGS_FORCED             = BITFLAG( 2 ), // ignore the interaction chance of the units
		ASSIGNFLAGS_NO_NEARBY_UNITS    = BITFLAG( 3 ), // don't look for other units around the unit
		ASSIGNFLAGS_SKIP_UNIT_CHECKS   = BITFLAG( 4 ), // skip AInteractionComponent::CanUnitsInteract
		ASSIGNFLAGS_OBEY_IGNORE_LIST   = BITFLAG( 5 ), // forced interactions ignore the ignore lists unless set
	};

	// Interactions that can be picked for a sequence, weighted by their chance.
	struct Candidates
	{
		struct Candidate
		{
			TINT   iInteraction; // 0x0
			TFLOAT fWeight;      // 0x4
		};

		Candidates();

		// NOTE: this is SetSize of T2DynamicVector<Candidate>
		void SetSize( TINT a_iSize );

		void       Add( TINT a_iInteraction, TFLOAT a_fWeight );
		void       Remove( Candidate* a_pCandidate );
		Candidate* Pick();
		TBOOL      IsEmpty();
		void       Clear();

		// Returns a random weight from 0 to a_fMaxWeight (excluding the epsilon)
		static TFLOAT GetRandomWeight( Toshi::TRandom* a_pRandom, TFLOAT a_fMaxWeight );

		// $Barnyard: FUNCTION 00401140
		static TBOOL IsWeightZero( TFLOAT a_fWeight ) { return a_fWeight <= Toshi::TMath::TFLOAT_EPSILON; }

		Toshi::TRandom*                   pRandom;       // 0x00
		TFLOAT                            fTotalWeight;  // 0x04
		Toshi::T2DynamicVector<Candidate> vecCandidates; // 0x08
	};

	// Free and active sequences.
	struct SequencePool
	{
		SequencePool();
		~SequencePool();

		// Moves a sequence from the free list to the active list.
		AInteractionSequence* Allocate();

		// Moves a sequence back to the free list.
		void Free( AInteractionSequence* a_pSequence );

		AInteractionSequence                   aSequences[ MAX_NUM_SEQUENCES ]; // 0x000
		Toshi::TNodeList<AInteractionSequence> llActive;                        // 0xe80
		Toshi::TNodeList<AInteractionSequence> llFree;                          // 0xe90
	};

	// Latches set by game code (SetPersistentLatch), saved with the game.
	struct PersistentLatchNames
	{
		void Clear();

		TINT  iNumNames;                                    // 0x000
		TCHAR aszNames[ MAX_NUM_PERSISTENT_LATCHES ][ 16 ]; // 0x004
	};

	// Name to value hash map of the persistent latches. The keys point into PersistentLatchNames.
	struct PersistentLatchMap
	{
		struct Entry
		{
			~Entry();

			const TCHAR* szName; // 0x0
			TINT         iValue; // 0x4
			Entry*       pNext;  // 0x8
		};

		struct Iterator
		{
			Iterator( PersistentLatchMap* a_pMap );
			Iterator( TINT a_iBucket, Entry* a_pEntry, PersistentLatchMap* a_pMap )
			    : iBucket( a_iBucket ), pEntry( a_pEntry ), pMap( a_pMap )
			{}

			void Next();

			TBOOL operator!=( const Iterator& a_rcOther ) const { return iBucket != a_rcOther.iBucket || pEntry != a_rcOther.pEntry || pMap != a_rcOther.pMap; }

			TINT                iBucket; // 0x0
			Entry*              pEntry;  // 0x4
			PersistentLatchMap* pMap;    // 0x8
		};

		~PersistentLatchMap();

		// NOTE: shared with the map of AUnitNameManager (the same template in retail)
		// $Barnyard: FUNCTION 00411db0
		Iterator End() { return Iterator( -1, TNULL, this ); }

		void  Clear();
		void  Set( const TCHAR* a_szName, TINT a_iValue );
		TBOOL Find( const TCHAR* a_szName, TINT& a_rValue ) const;

		TINT    iNumEntries; // 0x0
		TINT    iNumBuckets; // 0x4
		Entry** ppBuckets;   // 0x8
	};

public:
	AInteractionManager();
	~AInteractionManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Create() OVERRIDE;                        // +0x0c
	virtual TBOOL Reset() OVERRIDE;                         // +0x14
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x1c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x38

	// FSM natives
	TBOOL                 CanAssignInteraction();
	AInteractionSequence* AssignInteraction( AUnit* a_pUnit, Toshi::TPString8 a_strInteraction );
	AInteractionSequence* AssignInteractionForced( AUnit* a_pUnit, Toshi::TPString8 a_strInteraction );
	AInteractionSequence* AssignInteractionForcedObeyIgnoreList( AUnit* a_pUnit, Toshi::TPString8 a_strInteraction );
	void                  SetPersistentLatch( Toshi::TPString8 a_strName, TINT a_iValue );
	static TBOOL          ForceRequirementSatisfied( TINT a_iRequirement, TBOOL a_bSatisfied );

	// Starts an interaction with a_pUnit and the units around it.
	AInteractionSequence* AssignInteractionImpl( AUnit* a_pUnit, Toshi::TPString8 a_strInteraction, ASSIGNFLAGS a_uiFlags );

	// Starts an interaction with the given units.
	AInteractionSequence* AssignInteractionToUnits( AUnit** a_ppUnits, TINT a_iNumUnits, Toshi::TPString8 a_strInteraction, ASSIGNFLAGS a_uiFlags );

	// Starts an interaction with every interactive unit (up to 128).
	AInteractionSequence* AssignInteractionToAllUnits( Toshi::TPString8 a_strInteraction );

	// Value of a persistent latch, 0 if it's not set.
	TINT GetPersistentLatch( const TCHAR* a_szName );

	void SavePersistentLatches( AHackedQuickSave::Buffer* a_pBuffer );
	void LoadPersistentLatches( AHackedQuickSave::Buffer* a_pBuffer );

	// Priority of a priority type (Data/Interactions/prioritytypes.trb), -1 if unknown.
	TINT GetPriority( const Toshi::TPString8& a_rcPriorityType );

	TINT GetRequirementIndex( Toshi::TPString8 a_strName );
	TINT FindOrCreateRequirement( Toshi::TPString8 a_strName );

	// Resets the requirements forced by ForceRequirementSatisfied.
	void ResetForcedRequirements();

	// Finds a latch by its name ("not_" and "latch_" prefixes are parsed), adds it if it doesn't exist.
	ALatchRef GetLatch( Toshi::TPString8 a_strName );

	// Starts the interaction a_iInteraction with the units of a_pSequence.
	TBOOL StartInteraction( AInteractionSequence* a_pSequence, TINT a_iInteraction );

	// Collects the interactions that can be played by the units.
	Candidates* FindCandidates( AInteractionSequence* a_pSequence, TBOOL a_bStartOnly, AInteractionParticipants* a_pParticipants, Toshi::TPString8 a_strInteraction );
	Candidates* FindCandidates( AInteractionSequence* a_pSequence, TBOOL a_bStartOnly, Toshi::TPString8 a_strInteraction );

	// Picks one of the candidate interactions and starts it.
	TBOOL PickInteraction( AInteractionSequence* a_pSequence, AInteractionParticipants* a_pParticipants, Toshi::TPString8 a_strInteraction, TBOOL a_bTakeOverUnits );
	TBOOL PickInteraction( AInteractionSequence* a_pSequence, Toshi::TPString8 a_strInteraction );

	// Takes a free sequence and starts an interaction with the participants.
	AInteractionSequence* CreateSequence( AInteractionParticipants* a_pParticipants, Toshi::TPString8 a_strInteraction, TBOOL a_bTakeOverUnits );

	// Resets the sequence and moves it back to the free list.
	void EndSequence( AInteractionSequence* a_pSequence, TBOOL a_bPlayIdleAnim );

	TBOOL IsEnabled() const { return m_bEnabled; }
	TBOOL IsDebugEnabled() const { return m_bDebug; }
	TINT  GetStartLatch() const { return m_iStartLatch; }
	TINT  GetFinishLatch() const { return m_iFinishLatch; }

	AInteraction*            GetInteraction( TINT a_iIndex ) { return m_aInteractions[ a_iIndex ]; }
	AInteractionRequirement* GetRequirement( TINT a_iIndex ) { return m_aRequirements[ a_iIndex ]; }
	AInteractionActivity*    GetActivity( TINT a_iIndex ) { return m_aActivities[ a_iIndex ]; }

public:
	static AFunctionTable ms_oFunctionTable;

private:
	TBOOL LoadPriorityTypes();
	TBOOL LoadActivitySets( const TCHAR* a_szFileName, TINT a_iFirstActivity );
	TBOOL LoadInteractions( const TCHAR* a_szFileName, TINT a_iFirstActivity );
	TBOOL LoadRequirements( const PBProperties* a_pProperties );

#ifdef BARNYARD_LATEST
	// NOTE: not in the original. Applies the act 1 order of the latest version to the PC AllInteractions (the Wii
	// changes the "Act 1 Tier N - Complete" groups and the tier the quest givers wait for), see AQuestManager::LoadTierData
	void ApplyLatestAct1Order();
#endif

	// Allocates the activity scratch memory of every sequence.
	void AllocateActivityData();

	// Whether a_iInteraction should be added to the candidates; removes a candidate of
	// the same group that has a lower priority.
	TBOOL CheckCandidateGroup( Candidates* a_pCandidates, TINT a_iInteraction );

	static AInteractionRequirement* CreateRequirement( const PBProperties::PBProperty* a_pProperty );
	static AInteractionActivity*    CreateActivity( const PBProperties::PBProperty* a_pProperty );

private:
	TINT                                                               m_iUnk20;                // 0x0020, never used
	AInteractionEventManager*                                          m_pEventManager;         // 0x0024
	Toshi::TArray<Toshi::TPString8>                                    m_aLatchNames;           // 0x0028
	Toshi::TArray<AInteraction*>                                       m_aInteractions;         // 0x0038
	Toshi::TArray<AInteractionRequirement*>                            m_aRequirements;         // 0x0048
	Toshi::TArray<AInteractionActivity*>                               m_aActivities;           // 0x0058
	Toshi::T2DynamicVector<TINT>                                       m_vecStartInteractions;  // 0x0068, interactions that can start a sequence
	SequencePool                                                       m_oSequences;            // 0x007c
	AInteractionActivityMessageGroups*                                 m_pMessageGroups;        // 0x0f1c
	TINT                                                               m_iStartLatch;           // 0x0f20, "start"
	TINT                                                               m_iFinishLatch;          // 0x0f24, "finish"
	TBOOL                                                              m_bUnkf28;               // 0x0f28
	TBOOL                                                              m_bDebug;                // 0x0f29
	TBOOL                                                              m_bEnabled;              // 0x0f2a
	Candidates                                                         m_oCandidates;           // 0x0f2c
	Toshi::T2Map<Toshi::TPString8, TINT, Toshi::TPString8::Comparator> m_mapPriorityTypes;      // 0x0f48
	PersistentLatchNames                                               m_oPersistentLatchNames; // 0x0f60
	PersistentLatchMap                                                 m_oPersistentLatches;    // 0x1764
};

TSTATICASSERT( sizeof( AInteractionManager::SequencePool ) == 0xea0 );
TSTATICASSERT( sizeof( AInteractionManager::Candidates ) == 0x1c );
TSTATICASSERT( sizeof( AInteractionManager ) == 0x1770 );
