#pragma once
#include "AInteractionEvent.h"

//-----------------------------------------------------------------------------
// The player insulted a unit (created by name by the interactions). The units
// that witnessed it react to it for 3 minutes (APlayerInsultRequirement).
// TClass 0x0078be70 (version 1.0, size 0x2c, runtime creatable), vtable 0x0072ca08.
//-----------------------------------------------------------------------------
class AInteractionPlayerInsultEvent
    : public AInteractionEvent
{
public:
	TDECLARE_CLASS( AInteractionPlayerInsultEvent, AInteractionEvent );

	// How long an insult is remembered, in seconds (0x0072ca58)
	static constexpr TFLOAT MEMORY_DURATION = 180.0f;

public:
	AInteractionPlayerInsultEvent();
	~AInteractionPlayerInsultEvent();

	//-----------------------------------------------------------------------------
	// AInteractionEvent
	//-----------------------------------------------------------------------------
	virtual void                  Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x10
	virtual void                  Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x14
	virtual AInteractionEventText GetEventText( AUnit* a_pUnit ) OVERRIDE;               // +0x18
	virtual TBOOL                 Initialise( TINT a_iUnk ) OVERRIDE;                    // +0x1c
	virtual TBOOL                 Update( TFLOAT a_fDeltaTime ) OVERRIDE;                // +0x20
	virtual void                  SetId( TUINT a_uiId ) OVERRIDE;                        // +0x24, also remembers the time of the insult
	virtual void                  Reset() OVERRIDE;                                      // +0x28
	virtual void                  SetUnit( AUnit* a_pUnit ) OVERRIDE;                    // +0x34, lets the unit witness the insult
	virtual TBOOL                 IsWitness( AUnit* a_pUnit ) OVERRIDE { return TTRUE; } // +0x44, RETURNS_TRUE (00413920)

	// TRUE if the insult happened at most MEMORY_DURATION seconds ago
	TBOOL IsRecent();

private:
	TFLOAT m_fGlobalKnowledge; // 0x20, see AInteractionEvent::ComputeGlobalKnowledge
	TFLOAT m_fKnowledgeTimer;  // 0x24, the knowledge is computed every 15 seconds
	TFLOAT m_fInsultTime;      // 0x28, seconds of the system timer when the event was activated
};

TSTATICASSERT( sizeof( AInteractionPlayerInsultEvent ) == 0x2c );
