#pragma once
#include "AInteractionEvent.h"

class AUnitType;

//-----------------------------------------------------------------------------
// Units of a type are scared (see AFearRequirement).
// TClass AInteractionFearEvent, vtable 0x0072c928, size 0x2c.
//-----------------------------------------------------------------------------
class AInteractionFearEvent
    : public AInteractionEvent
{
public:
	TDECLARE_CLASS( AInteractionFearEvent, AInteractionEvent );

public:
	AInteractionFearEvent();
	~AInteractionFearEvent();

	//-----------------------------------------------------------------------------
	// AInteractionEvent
	//-----------------------------------------------------------------------------
	virtual void                  Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x10
	virtual void                  Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x14
	virtual AInteractionEventText GetEventText( AUnit* a_pUnit ) OVERRIDE;               // +0x18, folded with AInteractionGiftEvent's (005c7540)
	virtual TBOOL                 Initialise( TINT a_iUnk ) OVERRIDE;                    // +0x1c, folded with AInteractionGiftEvent's (005c6ed0)
	virtual TBOOL                 Update( TFLOAT a_fDeltaTime ) OVERRIDE;                // +0x20
	virtual void                  Reset() OVERRIDE;                                      // +0x28
	virtual void                  SetUnit( AUnit* a_pUnit ) OVERRIDE;                    // +0x34, folded with AInteractionGiftEvent's (005c6fd0)
	virtual TBOOL                 IsWitness( AUnit* a_pUnit ) OVERRIDE { return TTRUE; } // +0x44, RETURNS_TRUE (00413920)

	AUnitType* GetUnitType() const { return m_pUnitType; }

private:
	AUnitType* m_pUnitType;        // 0x20
	TFLOAT     m_fGlobalKnowledge; // 0x24, see AInteractionEvent::ComputeGlobalKnowledge
	TFLOAT     m_fKnowledgeTimer;  // 0x28, the knowledge is computed every 15 seconds
};

TSTATICASSERT( sizeof( AInteractionFearEvent ) == 0x2c );
