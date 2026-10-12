#pragma once
#include "AInteractionEvent.h"

class AQuest;

//-----------------------------------------------------------------------------
// A quest was completed (or failed), thrown by AQuestManager::RemoveQuest.
// The quest giver witnesses it. The event is kept after it expires.
//-----------------------------------------------------------------------------
class AInteractionQuestEvent
    : public AInteractionEvent
{
public:
	TDECLARE_CLASS( AInteractionQuestEvent, AInteractionEvent );

public:
	AInteractionQuestEvent();
	~AInteractionQuestEvent();

	//-----------------------------------------------------------------------------
	// AInteractionEvent
	//-----------------------------------------------------------------------------
	virtual void                  Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x10
	virtual void                  Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x14
	virtual AInteractionEventText GetEventText( AUnit* a_pUnit ) OVERRIDE;               // +0x18, folded with AInteractionGiftEvent's (005c7540)
	virtual TBOOL                 Initialise( TINT a_iUnk ) OVERRIDE;                    // +0x1c, folded with AInteractionGiftEvent's (005c6ed0)
	virtual TBOOL                 Update( TFLOAT a_fDeltaTime ) OVERRIDE;                // +0x20
	virtual void                  Reset() OVERRIDE;                                      // +0x28
	virtual void                  SetUnitRef( AUnitRef* a_pUnitRef ) OVERRIDE;           // +0x30, the quest giver
	virtual void                  SetUnit( AUnit* a_pUnit ) OVERRIDE;                    // +0x34, the quest giver
	virtual TBOOL                 IsWitness( AUnit* a_pUnit ) OVERRIDE { return TTRUE; } // +0x44, RETURNS_TRUE (00413920)
	virtual TBOOL                 IsPersistent() OVERRIDE { return TTRUE; }              // +0x48, RETURNS_TRUE (0046cde0)

	void SetQuest( AQuest* a_pQuest ) { m_pQuest = a_pQuest; }
	void SetCompleted( TBOOL a_bCompleted ) { m_bCompleted = a_bCompleted; }

	AQuest*      GetQuest() const { return m_pQuest; }
	TBOOL        IsCompleted() const { return m_bCompleted; }
	TBOOL        WasThanked() const { return m_bThanked; }
	const TCHAR* GetGiverTypeName() const { return m_szGiverTypeName; }

private:
	AQuest*      m_pQuest;          // 0x20
	TBOOL        m_bCompleted;      // 0x24
	TBOOL        m_bThanked;        // 0x25
	const TCHAR* m_szGiverTypeName; // 0x28, name of the type of the quest giver
};

TSTATICASSERT( sizeof( AInteractionQuestEvent ) == 0x2c );
