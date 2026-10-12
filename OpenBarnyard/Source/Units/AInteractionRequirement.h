#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TPString8.h>
#include <Plugins/PPropertyParser/PBProperties.h>

#include "AInteraction.h"

//-----------------------------------------------------------------------------
// Unit type names for the primary, secondary and auxiliary roles, filled by
// AInteractionRequirement::GetUnitTypeNames.
// NOTE: Only the method that isn't inlined is reproduced.
//-----------------------------------------------------------------------------
struct AInteractionUnitTypeNames
{
	// Sets the name of a role, CATEGORY_ALL sets the name of every role.
	void Set( AInteractionSequence::CATEGORY a_eCategory, const Toshi::TPString8& a_rcName );

	TUINT            uiUnk0;                                    // 0x00
	Toshi::TPString8 astrNames[ AInteractionRoles::NUM_ROLES ]; // 0x04
};

//-----------------------------------------------------------------------------
// A comparison of the requirements ("compare": "all", "lessthan", "lessthanequal",
// "equal", "notequal", "greaterthan", "greaterthanequal").
//-----------------------------------------------------------------------------
struct AInteractionComparison
{
	using OPERATOR = TUINT8;
	enum OPERATOR_ : OPERATOR
	{
		OPERATOR_ALL,
		OPERATOR_LESSTHAN,
		OPERATOR_LESSTHANEQUAL,
		OPERATOR_EQUAL,
		OPERATOR_NOTEQUAL,
		OPERATOR_GREATERTHAN,
		OPERATOR_GREATERTHANEQUAL,
		OPERATOR_NUMOF,
	};

	// $Barnyard: FUNCTION 0059e480
	AInteractionComparison( OPERATOR a_eOperator )
	    : eOperator( a_eOperator )
	{}

	// Whether a_fValue compares to a_fReference
	TBOOL Compare( TFLOAT a_fValue, TFLOAT a_fReference ) const;

	// Whether a_iValue compares to a_iReference
	TBOOL Compare( TINT a_iValue, TINT a_iReference ) const;

	// $Barnyard: FUNCTION 0059e490
	TBOOL SetOperator( OPERATOR a_eOperator )
	{
		eOperator = a_eOperator;
		return TTRUE;
	}

	// Sets the operator from its name, TFALSE if there's no such operator
	TBOOL SetOperator( const Toshi::TPString8& a_rcName );

	OPERATOR eOperator; // 0x0
};

//-----------------------------------------------------------------------------
// A condition of an interaction ("requirements" of Data/Interactions/*.trz).
// Evaluate decides which participants can take which role (m_oRoles) and with
// which weight. Subclasses are created by name, see AInteractionManager::CreateRequirement.
//-----------------------------------------------------------------------------
class AInteractionRequirement
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AInteractionRequirement, Toshi::TObject );

	friend class AInteraction;
	friend class AInteractionManager;
	friend class AGroupRequirement;

	// See AInteractionManager::ForceRequirementSatisfied
	using FORCEDSTATE = TINT;
	enum FORCEDSTATE_ : FORCEDSTATE
	{
		FORCEDSTATE_SATISFIED     = 0,
		FORCEDSTATE_NOT_SATISFIED = 1,
		FORCEDSTATE_NONE          = 2,
	};

public:
	AInteractionRequirement();
	~AInteractionRequirement();

	virtual TBOOL Create( const PBProperties* a_pProperties );                                                       // +0x0c
	virtual void  OnSequenceStart( AInteractionParticipants* a_pParticipants, AInteractionSequence* a_pSequence ) {} // +0x10, DOES_NOTHING
	virtual void  OnSequenceEnd( AInteractionSequence* a_pSequence ) {}                                              // +0x14, DOES_NOTHING
	virtual void  GetUnitTypeNames( AInteractionUnitTypeNames* a_pNames ) {}                                         // +0x18, DOES_NOTHING
	virtual void  Evaluate( AInteractionParticipants* a_pParticipants ) = 0;                                         // +0x1c

	const Toshi::TPString8& GetName() const { return m_strName; }

	void SetForcedState( FORCEDSTATE a_eForcedState ) { m_eForcedState = a_eForcedState; }

protected:
	// Lets the unit take the role of a_eCategory (or every role for CATEGORY_ALL) if a_bAllowed, honours "invert".
	void SetUnitAllowed( AInteractionSequence::CATEGORY a_eCategory, TUINT a_uiUnit, TBOOL a_bAllowed );

protected:
	AInteractionRoles              m_oRoles;           // 0x04
	TUINT                          m_uiParticipantsId; // 0x38, id of the participants m_oRoles was evaluated for
	Toshi::TPString8               m_strName;          // 0x3c, "name", only kept while loading
	AInteractionSequence::CATEGORY m_eCategory;        // 0x40, "category"
	TBOOL                          m_bInvert;          // 0x44, "invert"
	FORCEDSTATE                    m_eForcedState;     // 0x48
};

TSTATICASSERT( sizeof( AInteractionRequirement ) == 0x4c );
