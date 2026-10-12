#pragma once
#include "AInteractionEvent.h"
#include "AUnitRef.h"

//-----------------------------------------------------------------------------
// The coyotes are attacking. Remembers up to MAX_NUM_UNITS units that took part.
// TClass AInteractionCoyoteEvent, vtable 0x0072c6b8, size 0x78.
//-----------------------------------------------------------------------------
class AInteractionCoyoteEvent
    : public AInteractionEvent
{
public:
	TDECLARE_CLASS( AInteractionCoyoteEvent, AInteractionEvent );

	static constexpr TINT MAX_NUM_UNITS = 5;

public:
	AInteractionCoyoteEvent();
	~AInteractionCoyoteEvent();

	//-----------------------------------------------------------------------------
	// AInteractionEvent
	//-----------------------------------------------------------------------------
	virtual void                  Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;               // +0x10
	virtual void                  Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;               // +0x14
	virtual AInteractionEventText GetEventText( AUnit* a_pUnit ) OVERRIDE;                            // +0x18
	virtual TBOOL                 Initialise( TINT a_iUnk ) OVERRIDE;                                 // +0x1c, folded with AInteractionGiftEvent's (005c6ed0)
	virtual TBOOL                 Update( TFLOAT a_fDeltaTime ) OVERRIDE;                             // +0x20, folded with AInteractionGiftEvent's (005c5170)
	virtual void                  Reset() OVERRIDE;                                                   // +0x28
	virtual void                  SetUnit( AUnit* a_pUnit ) OVERRIDE;                                 // +0x34
	virtual TFLOAT                GetGlobalKnowledgeOfEvent() OVERRIDE { return m_fGlobalKnowledge; } // +0x3c, folded getter 0056d180

	// Remembers the unit (if there's room for it) and lets it know about the event
	virtual void AddUnit( AUnit* a_pUnit ); // +0x50

private:
	TFLOAT   m_fGlobalKnowledge;             // 0x20, see AInteractionEvent::ComputeGlobalKnowledge
	TFLOAT   m_fKnowledgeTimer;              // 0x24, the knowledge is computed every 15 seconds
	AUnitRef m_aUnits[ MAX_NUM_UNITS ];      // 0x28
	TINT     m_aUnitStates[ MAX_NUM_UNITS ]; // 0x64, never read, reset to 0
};

TSTATICASSERT( sizeof( AInteractionCoyoteEvent ) == 0x78 );
