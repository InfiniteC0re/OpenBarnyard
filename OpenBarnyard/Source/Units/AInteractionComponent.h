#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/TPString8.h>
#include <Toshi/TRefCounted.h>

#include "AUnitComponent.h"
#include "AUnitProxy.h"

class AInteractionComponentData;
class AInteractionComponentManager;
class AInteractionSequence;
class ASpeechActivity;

// Lets a unit take part in interactions (AInteractionSequence). Instances are allocated
// from AInteractionComponentManager's pool. While a component is not in a sequence it
// sits in the manager's list, which counts the unit's idle time and asks
// AInteractionManager for an interaction once the unit has been idle long enough.
class AInteractionComponent
    : public AUnitComponent
    , public Toshi::T2DList<AInteractionComponent>::Node
{
public:
	TDECLARE_CLASS( AInteractionComponent, AUnitComponent );

	friend AInteractionComponentManager;

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                      = 0,
		FLAGS_ALLOW_NORMAL_INTERACTIONS = BITFLAG( 0 ), // FSM "SetAllowNormalInteractions", idle time is only counted while set
		FLAGS_ALLOW_CONVERSATION        = BITFLAG( 1 ), // FSM "SetAllowConversation"
		FLAGS_INTERACTIONS_DISABLED     = BITFLAG( 2 ), // FSM "SetInteractionsDisabled"
	};

	static constexpr TINT MAX_NUM_IGNORED_UNITS = 6;
	static constexpr TINT NUM_INTERVALS         = 3;

	// Unit this component doesn't start interactions with for a while (see AddToIgnoreList).
	struct IgnoredUnit
	{
		// NOTE: the constructor is folded with other two-dword zeroing constructors (005c8ef0)
		IgnoredUnit()
		    : fTime( 0.0f )
		{}

		Toshi::TRef<AUnitProxy> oUnit; // 0x00
		TFLOAT                  fTime; // 0x04, seconds left
	};

	// NOTE: retail uses a fixed-capacity vector template here (count first, then the elements);
	// only the two methods that weren't inlined are reproduced.
	struct IgnoreList
	{
		IgnoreList()
		    : iNumElements( 0 )
		{}

		// Releases every element.
		void Clear();

		// Erases an element keeping the order of the others, returns the index of the next one.
		TINT Erase( TINT a_iIndex );

		TINT        iNumElements;                       // 0x00
		IgnoredUnit aElements[ MAX_NUM_IGNORED_UNITS ]; // 0x04
	};

public:
	AInteractionComponent();
	~AInteractionComponent();

	virtual AFunctionTable*        GetFunctionTable() OVERRIDE;                          // +0x0c
	virtual AUnitComponentManager* GetManager() OVERRIDE;                                // +0x10
	virtual void                   Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x14, writes the "InteractionComponent" save block
	virtual void                   Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE; // +0x18, reads the "InteractionComponent" save block

	// FSM natives
	TBOOL  IsSuitableForInteraction();
	TBOOL  IsSuitableForForcedInteraction();
	void   ResetTimeSinceLastInteraction();
	void   SetAllowNormalInteractions( TBOOL a_bAllow );
	TBOOL  IsAllowNormalInteractions();
	void   ResetForcedInteraction();
	void   SetForcedInteraction( const Toshi::TPString8& a_rcInteraction, const Toshi::TPString8& a_rcArgument, TFLOAT a_fTime );
	void   ResetIgnoreList();
	TFLOAT GetTimeSinceLastInteraction();
	void   SetAllowConversation( TBOOL a_bAllow );
	TBOOL  IsAllowConversation();
	void   SetInteractionsDisabled( TBOOL a_bDisabled );

	TBOOL IsInteractionsDisabled();

	// Adds the sequence's start to the interval history.
	void RecordInteraction();

	// Number of the last recorded interactions that started within a_fTime seconds.
	TINT GetNumInteractionsWithin( TFLOAT a_fTime );

	// Chance (0..1) that the unit agrees to a spontaneous interaction.
	TFLOAT GetInteractionChance();

	TBOOL IsUnitIgnored( AUnit* a_pUnit );

	// Keeps a_pUnit out of interactions with this unit for 15 seconds.
	void AddToIgnoreList( AUnit* a_pUnit );

	// Called by AInteractionSequence when the unit joins it.
	void SetSequence( AInteractionSequence* a_pSequence );

	// Leaves the current sequence and returns to the manager's list.
	void LeaveSequence( TBOOL a_bPlayIdleAnim );

	// Whether the component is in a sequence that is still running.
	TBOOL IsInSequence();

	// Finds the speech activity of the current sequence.
	TBOOL GetSpeechActivity( ASpeechActivity** a_ppActivity );

	AInteractionSequence*   GetSequence() const { return m_pSequence; }
	TFLOAT                  GetInterval() const { return m_fInterval; }
	const Toshi::TPString8& GetForcedInteraction() const { return m_strForcedInteraction; }
	const Toshi::TPString8& GetForcedInteractionArg() const { return m_strForcedInteractionArg; }

	static TBOOL IsUnitSuitableForInteraction( AUnit* a_pUnit, TBOOL a_bForced );
	static TBOOL IsUnitInteractive( AUnit* a_pUnit );
	static TBOOL CanUnitsInteract( AUnit* a_pUnit, AUnit* a_pOther, TBOOL a_bIgnoreIgnoreList );
	static TBOOL CanAssignInteraction( AUnit* a_pUnit, AUnit* a_pOther, TBOOL a_bForced, TBOOL a_bSkipUnitChecks, TBOOL a_bIgnoreIgnoreList );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	// Advances the timers, returns TTRUE when the unit wants an interaction.
	TBOOL UpdateTimers( TFLOAT a_fDeltaTime );

	// Asks AInteractionManager for an interaction (the forced one if set).
	TBOOL TryAssignInteraction();

private:
	TFLOAT                m_fTimeSinceLastInteraction;    // 0x18
	TFLOAT                m_fInterval;                    // 0x1c, time since RecordInteraction
	TFLOAT                m_fReadyTime;                   // 0x20, time spent idle after the 5 second cooldown
	AInteractionSequence* m_pSequence;                    // 0x24
	TFLOAT                m_fForcedInteractionTime;       // 0x28
	Toshi::TPString8      m_strForcedInteractionArg;      // 0x2c
	Toshi::TPString8      m_strForcedInteraction;         // 0x30
	IgnoreList            m_oIgnoreList;                  // 0x34
	FLAGS                 m_uiFlags;                      // 0x68
	TFLOAT                m_afIntervals[ NUM_INTERVALS ]; // 0x6c, ring buffer of the last intervals, -1.0f if unused
	TINT                  m_iIntervalIndex;               // 0x78
};

TSTATICASSERT( sizeof( AInteractionComponent::IgnoredUnit ) == 0x8 );
TSTATICASSERT( sizeof( AInteractionComponent ) == 0x7c );
