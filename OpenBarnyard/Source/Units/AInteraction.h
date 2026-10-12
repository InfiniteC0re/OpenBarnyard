#pragma once
#include <Toshi/TArray.h>
#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PBProperties.h>

#include "AInteractionSequence.h"

class AUnit;
class AInteractionActivity;

//-----------------------------------------------------------------------------
// Which units can take which role of an interaction, filled by its requirements.
// Each mask has a bit for every unit of AInteractionParticipants.
//-----------------------------------------------------------------------------
struct AInteractionRoles
{
	static constexpr TINT NUM_ROLES = 3; // primary, secondary, auxiliary (see AInteractionSequence::CATEGORY)
	static constexpr TINT NUM_WORDS = AInteractionParticipants::MAX_NUM_UNITS / 32;

	// NOTE: inlined, only the masks are cleared
	AInteractionRoles()
	{
		for ( TINT i = 0; i < NUM_ROLES; i++ )
		{
			for ( TINT k = 0; k < NUM_WORDS; k++ )
				aauiMasks[ i ][ k ] = 0;
		}
	}

	// Lets every unit take every role.
	void Reset();

	// Lets every unit take every role, only the initiator (if any) can be the primary unit.
	void Reset( const AInteractionParticipants* a_pParticipants );

	// Lets no unit take any role.
	void Clear();

	// Keeps only what both allow.
	void Combine( const AInteractionRoles& a_rcOther );

	// Allows what either allows, keeps the higher weight.
	void Merge( const AInteractionRoles& a_rcOther );

	// Picks a unit for each role (the roles after the auxiliary one use its mask) without using a unit twice.
	// Returns the number of units picked, a_iMaxUnits if every role was given a unit.
	TINT AssignUnits( TINT* a_pUnits, TINT a_iNumUnits, TINT a_iMinUnits, TINT a_iMaxUnits );

	// Takes every role away from the unit.
	void RemoveUnit( TUINT a_uiUnit );

	// Lets the unit take the role or takes it away.
	void SetUnit( TINT a_iRole, TUINT a_uiUnit, TBOOL a_bValue ) { SetBit( aauiMasks[ a_iRole ], a_uiUnit, a_bValue ); }

	// NOTE: the Set method of a 128 bit set type in the original, shared with other code
	// $Barnyard: FUNCTION 00430070
	static void SetBit( TUINT* a_pWords, TUINT a_uiBit, TBOOL a_bValue )
	{
		TUINT uiBit = 1 << ( a_uiBit & 31 );

		if ( a_bValue )
			a_pWords[ a_uiBit >> 5 ] |= uiBit;
		else
			a_pWords[ a_uiBit >> 5 ] &= ~uiBit;
	}

	TFLOAT fWeight;                             // 0x00
	TUINT  aauiMasks[ NUM_ROLES ][ NUM_WORDS ]; // 0x04
};

TSTATICASSERT( sizeof( AInteractionRoles ) == 0x34 );

//-----------------------------------------------------------------------------
// An interaction loaded from Data/Interactions/*.trz: the latches it needs and
// sets, its requirements (indices into AInteractionManager's requirements) and its
// activities (indices into AInteractionManager's activities). Played by an
// AInteractionSequence.
//-----------------------------------------------------------------------------
class AInteraction
{
public:
	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                  = 0,
		FLAGS_INTERRUPTIBLE         = BITFLAG( 0 ),  // "interruptible", units in the sequence can still start other interactions
		FLAGS_UNK1                  = BITFLAG( 1 ),  // the interaction can't be picked while set
		FLAGS_STOP_ANIMATIONS       = BITFLAG( 2 ),  // "stopanimations", default on
		FLAGS_OVERRIDEABLE          = BITFLAG( 3 ),  // "overrideable"
		FLAGS_HAS_WALK_ACTIVITY     = BITFLAG( 4 ),  // set if one of the activities says so (AInteractionActivity +0x38)
		FLAGS_DIALOGUE_NEXT         = BITFLAG( 5 ),  // "dialoguenext"
		FLAGS_PRIORITY_MOD          = BITFLAG( 6 ),  // the priority is computed by UpdatePriority
		FLAGS_PRIORITY_MOD_CATEGORY = BITFLAG( 7 ),  // "prioritymodcategory", the priority depends on the relations of a unit
		FLAGS_PRIORITY_CHANCE_MOD   = BITFLAG( 8 ),  // "prioritychancemod", a random value is added to the priority
		FLAGS_PAUSE_PLAYER_STEERING = BITFLAG( 9 ),  // "pauseplayersteering", default on
		FLAGS_SWAP_PRIMARY          = BITFLAG( 10 ), // "nextprimary" isn't "keep", the next interaction of the sequence starts with the secondary unit
		FLAGS_MISSION_CONVERSATION  = BITFLAG( 12 ), // "semantic" is "mission conversation"
		FLAGS_MISSION_OBJECTIVE     = BITFLAG( 13 ), // "semantic" is "mission objective"
	};

	// What happens when there are less units than "numunits" ("minunitfailbehaviour").
	using MINUNITFAILBEHAVIOUR = TUINT8;
	enum MINUNITFAILBEHAVIOUR_ : MINUNITFAILBEHAVIOUR
	{
		MINUNITFAILBEHAVIOUR_CONTINUE,
		MINUNITFAILBEHAVIOUR_FINISH,
		MINUNITFAILBEHAVIOUR_ASSERT,
		MINUNITFAILBEHAVIOUR_NUMOF
	};

	static constexpr TINT MAX_NUM_REQUIREMENTS = 7;

	// Modifies the priority of interactions with FLAGS_PRIORITY_MOD (see UpdatePriority).
	// NOTE: Allocated with TMalloc and zeroed by Load.
	struct PriorityMods
	{
		TINT16  aiRelationMods[ 3 ]; // 0x00, "prioritymod": friends, neither, not friends
		TUINT16 uiCategory;          // 0x06, "prioritymodcategory", unit whose relations are checked
		TINT16  aiRandomRange[ 2 ];  // 0x08, "prioritychancemod": min, max
	};

public:
	AInteraction();
	virtual ~AInteraction(); // +0x00

	// Loads the interaction from its properties, a_iFirstActivity is the index of the first activity of its activity set.
	TBOOL Load( const Toshi::TPString8& a_rcFileName, const PBProperties* a_pProperties, TINT a_iFirstActivity );

	// Whether the latches allow the interaction to start.
	TBOOL CanStart( AInteractionLatches* a_pLatches );

	AInteractionActivity* GetActivity( TINT a_iIndex );

	// Tells the requirements that a sequence starts playing the interaction.
	void OnSequenceStart( AInteractionParticipants* a_pParticipants, AInteractionSequence* a_pSequence );

	// Tells the requirements that a sequence stops playing the interaction.
	void OnSequenceEnd( AInteractionSequence* a_pSequence );

	// Computes the current priority for the units picked for the roles.
	void UpdatePriority( AInteractionParticipants* a_pParticipants, TINT* a_pUnits, TINT a_iNumUnits );

	// Asks the requirements which units can take which role, a_pRoles->fWeight becomes the chance of the interaction.
	void EvaluateRequirements( AInteractionParticipants* a_pParticipants, AInteractionRoles* a_pRoles );

	// Picks the units that take part in the interaction, returns their number.
	TINT SelectUnits( TINT* a_pUnits, AInteractionParticipants* a_pParticipants, AInteractionRoles* a_pRoles );

	// Returns the first (localised) text one of the activities provides (see AInteractionActivity::GetText).
	TWCHAR* GetActivityText();

	TBOOL IsInterruptible() const { return ( m_uiFlags & FLAGS_INTERRUPTIBLE ) != 0; }
	TBOOL ShouldStopAnimations() const { return ( m_uiFlags & FLAGS_STOP_ANIMATIONS ) != 0; }

	// Lets one more unit take part in the interaction (see AAddUnitsActivity).
	void IncreaseMaxUnits() { m_iMaxUnits++; }

	// Restores the maximum number of units read from "numunits".
	void ResetMaxUnits()
	{
		if ( m_iMaxUnits != m_iDefaultMaxUnits )
			m_iMaxUnits = m_iDefaultMaxUnits;
	}

	const Toshi::TPString8& GetUnk4() const { return m_strUnk4; }
	const Toshi::TPString8& GetInteractionSet() const { return m_strInteractionSet; }
	const ALatchRefList&    GetPreconditions() const { return m_aPreconditions; }
	const ALatchRefList&    GetPostconditions() const { return m_aPostconditions; }
	const Toshi::TPString8& GetTrigger() const { return m_strTrigger; }
	const Toshi::TPString8& GetEndTrigger() const { return m_strEndTrigger; }
	TFLOAT                  GetChance() const { return m_fChance; }
	TINT                    GetPriority() const { return m_iPriority; }
	TINT                    GetCurrentPriority() const { return m_iCurrentPriority; }
	FLAGS                   GetFlags() const { return m_uiFlags; }
	TINT                    GetNumActivities() const { return m_aActivities.Size(); }
	MINUNITFAILBEHAVIOUR    GetMinUnitFailBehaviour() const { return m_eMinUnitFailBehaviour; }
	TINT                    GetMinUnits() const { return m_iMinUnits; }
	TINT                    GetMaxUnits() const { return m_iMaxUnits; }
	TINT                    GetDisplayOrder() const { return m_iDisplayOrder; }
	PBPropertyLocaleString  GetHudText() const { return m_oHudText; }

	// Reads a list of latch names.
	static TBOOL LoadLatches( ALatchRefList* a_pLatches, const PBPropertyValueArray* a_pArray );

	// Creates the activities of an activity set.
	static TBOOL LoadActivitySet( Toshi::TArray<AInteractionActivity*>* a_pActivities, const PBProperties* a_pProperties );

	// Reads the activity indices of an interaction (arrays of indices into the activity set).
	static TBOOL LoadActivities( Toshi::TArray<AInteractionActivity*>* a_pActivities, const PBProperties* a_pProperties, TINT16 a_iFirstActivity, Toshi::TArray<TUINT16>* a_pIndices );

private:
	// Reads the requirement names.
	TBOOL LoadRequirements( const PBPropertyValueArray* a_pArray );

private:
	Toshi::TPString8       m_strUnk4;                                 // 0x04, never set
	Toshi::TPString8       m_strInteractionSet;                       // 0x08, "interactionset", only one interaction of a set can be a candidate
	ALatchRefList          m_aPreconditions;                          // 0x0c, "preconditions"
	ALatchRefList          m_aPostconditions;                         // 0x1c, "postconditions"
	Toshi::TPString8       m_strTrigger;                              // 0x2c, "trigger", the name the interaction is started by
	Toshi::TPString8       m_strEndTrigger;                           // 0x30, "endtrigger", interaction that follows this one
	TFLOAT                 m_fChance;                                 // 0x34, "chance"
	TINT                   m_iPriority;                               // 0x38, "priority"
	TINT                   m_iCurrentPriority;                        // 0x3c, see UpdatePriority
	FLAGS                  m_uiFlags;                                 // 0x40
	Toshi::TArray<TUINT16> m_aActivities;                             // 0x44, "activities"
	TUINT16                m_auiRequirements[ MAX_NUM_REQUIREMENTS ]; // 0x54, "requirements"
	MINUNITFAILBEHAVIOUR   m_eMinUnitFailBehaviour;                   // 0x62, "minunitfailbehaviour"
	TINT8                  m_iMinUnits;                               // 0x63, "numunits"
	TINT8                  m_iMaxUnits;                               // 0x64, "numunits", can be changed while the interaction is played
	TINT8                  m_iDefaultMaxUnits;                        // 0x65
	TINT8                  m_iDisplayOrder;                           // 0x66, "displayorder"
	TINT8                  m_iNumRequirements;                        // 0x67
	PBPropertyLocaleString m_oHudText;                                // 0x68, "hudtext"
	PriorityMods*          m_pPriorityMods;                           // 0x6c
};

TSTATICASSERT( sizeof( AInteraction::PriorityMods ) == 0xc );
TSTATICASSERT( sizeof( AInteraction ) == 0x70 );
