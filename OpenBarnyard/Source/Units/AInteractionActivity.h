#pragma once
#include <Toshi/TObject.h>
#include <Toshi/TArray.h>
#include <Toshi/TGlobalEmitter.h>
#include <Plugins/PPropertyParser/PBProperties.h>

#include "AInteractionSequence.h"

class AUnit;
class AInteraction;
class AInteractionActivity;

//-----------------------------------------------------------------------------
// Thrown through TGlobalEmitter while the activities of a sequence are played
// (listener list at 0x0077dd70).
//-----------------------------------------------------------------------------
class AInteractionActivityEvent
    : public Toshi::TGlobalEvent<AInteractionActivityEvent>
{
public:
	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_NONE            = 0,
		TYPE_STARTED         = 1, // the activity has been started
		TYPE_FINISHED        = 2, // the activity is over, the next one follows
		TYPE_SEQUENCE_FINISH = 3, // the activity finishes the sequence
	};

public:
	AInteractionActivityEvent( AInteractionSequence* a_pSequence )
	    : m_pSequence( a_pSequence )
	    , m_pActivity( TNULL )
	    , m_eType( TYPE_NONE )
	{}

	AInteractionSequence* GetSequence() const { return m_pSequence; }
	AInteractionActivity* GetActivity() const { return m_pActivity; }
	TYPE                  GetType() const { return m_eType; }

	void Set( AInteractionActivity* a_pActivity, TYPE a_eType )
	{
		m_pActivity = a_pActivity;
		m_eType     = a_eType;
	}

private:
	AInteractionSequence* m_pSequence; // 0x04
	AInteractionActivity* m_pActivity; // 0x08
	TYPE                  m_eType;     // 0x0c
};

//-----------------------------------------------------------------------------
// A step of an interaction ("activities" of Data/Interactions/*.trz, the
// activity sets are in ActivitySets1). The sequence plays the activities of its
// interaction one after the other (see UpdateActivities). Subclasses are created
// by name, see AInteractionManager::CreateActivity.
//-----------------------------------------------------------------------------
class AInteractionActivity
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AInteractionActivity, Toshi::TObject );

	// What OnStart / Update return
	using RESULT = TINT;
	enum RESULT_ : RESULT
	{
		RESULT_DONE            = 0, // the activity is over, the next one follows
		RESULT_RUNNING         = 1,
		RESULT_FINISH_SEQUENCE = 2,
	};

public:
	AInteractionActivity();
	~AInteractionActivity();

	virtual TBOOL  Create( const PBProperties* a_pProperties );                         // +0x0c
	virtual void   GetDataSize( TINT* a_pSize );                                        // +0x10, size of the scratch memory the activity needs in the sequence
	virtual TBOOL  GetText( TWCHAR** a_ppText, TBOOL a_bKeep ) { return TFALSE; }       // +0x14, RETURNS_ZERO, a localised text of the activity (see AInteraction::GetActivityText), a_bKeep: use it when the activity is played
	virtual RESULT OnStart( AInteractionSequence* a_pSequence ) { return RESULT_DONE; } // +0x18, RETURNS_ZERO
	virtual RESULT Update( AInteractionSequence* a_pSequence ) { return RESULT_DONE; }  // +0x1c, RETURNS_ZERO
	virtual TFLOAT Unk_0x20() { return 0.0f; }                                          // +0x20, returns the 0.0f at 0x00716c44
	virtual AUnit* GetActiveUnit( AInteractionSequence* a_pSequence );                  // +0x24, unit of AInteractionSequence::CATEGORY_ACTIVE
	virtual AUnit* GetTargetUnit( AInteractionSequence* a_pSequence ) { return TNULL; } // +0x28, RETURNS_ZERO, unit of AInteractionSequence::CATEGORY_TARGET
	virtual TBOOL  Unk_0x2c( void* a_pUnk ) { return TFALSE; }                          // +0x2c, RETURNS_FALSE
	virtual TBOOL  CanBeSkipped() { return TFALSE; }                                    // +0x30, RETURNS_FALSE, a finished activity ends a forced update (see UpdateActivities)
	virtual TBOOL  IsOverrideable() { return TTRUE; }                                   // +0x34, RETURNS_TRUE, see AInteraction::FLAGS_OVERRIDEABLE
	virtual TBOOL  IsWalkActivity() { return TFALSE; }                                  // +0x38, RETURNS_FALSE, see AInteraction::FLAGS_HAS_WALK_ACTIVITY

	AInteractionSequence::CATEGORY GetCategory() const { return m_eCategory; }

	// Plays the activities (of a_pInteraction, or a_pActivities if it's TNULL) from a_pState on.
	// Returns RESULT_DONE once every activity is over (or a skippable one is over while a_bForced),
	// RESULT_FINISH_SEQUENCE if an activity finishes the sequence and RESULT_RUNNING otherwise.
	static RESULT __stdcall UpdateActivities( AInteraction* a_pInteraction, Toshi::TArray<AInteractionActivity*>* a_pActivities, AInteractionSequence* a_pSequence, AInteractionActivityState* a_pState, TFLOAT a_fDeltaTime, TBOOL a_bForced );

protected:
	AInteractionSequence::CATEGORY m_eCategory; // 0x04, "category"
};

TSTATICASSERT( sizeof( AInteractionActivity ) == 0x8 );
