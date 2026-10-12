#pragma once
#include "AInteractionEvent.h"

#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// A unit gave items to another one (see AGiftActivity). Both units witness
// it, AGiftQuestElement waits for the player to give a gift this way.
//-----------------------------------------------------------------------------
class AInteractionGiftEvent
    : public AInteractionEvent
{
public:
	TDECLARE_CLASS( AInteractionGiftEvent, AInteractionEvent );

public:
	AInteractionGiftEvent();
	~AInteractionGiftEvent();

	//-----------------------------------------------------------------------------
	// AInteractionEvent
	//-----------------------------------------------------------------------------
	virtual void                  Save( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x10
	virtual void                  Load( AHackedQuickSave::Buffer* a_pBuffer ) OVERRIDE;  // +0x14
	virtual AInteractionEventText GetEventText( AUnit* a_pUnit ) OVERRIDE;               // +0x18
	virtual TBOOL                 Initialise( TINT a_iUnk ) OVERRIDE;                    // +0x1c
	virtual TBOOL                 Update( TFLOAT a_fDeltaTime ) OVERRIDE;                // +0x20
	virtual void                  Reset() OVERRIDE;                                      // +0x28
	virtual void                  SetUnit( AUnit* a_pUnit ) OVERRIDE;                    // +0x34, lets the unit witness the gift
	virtual TBOOL                 IsWitness( AUnit* a_pUnit ) OVERRIDE { return TTRUE; } // +0x44, RETURNS_TRUE (00413920)

	// The giver and the receiver witness the gift
	void SetGiver( AUnit* a_pUnit );
	void SetReceiver( AUnit* a_pUnit );

	// $Barnyard: FUNCTION 005c70d0
	void SetGiftType( const Toshi::TPString8& a_rcGiftType ) { m_strGiftType = a_rcGiftType; }
	// $Barnyard: FUNCTION 005c6f00
	void SetCount( TINT a_iCount ) { m_iCount = a_iCount; }

	const TCHAR* GetGiverTypeName() const { return m_szGiverTypeName; }
	const TCHAR* GetReceiverTypeName() const { return m_szReceiverTypeName; }
	// $Barnyard: FUNCTION 005cff10
	Toshi::TPString8 GetGiftType() const { return m_strGiftType; }
	TINT             GetCount() const { return m_iCount; }

private:
	TFLOAT           m_fGlobalKnowledge;   // 0x20, see AInteractionEvent::ComputeGlobalKnowledge
	TFLOAT           m_fKnowledgeTimer;    // 0x24, the knowledge is computed every 15 seconds
	const TCHAR*     m_szGiverTypeName;    // 0x28, name of the type of the giver
	const TCHAR*     m_szReceiverTypeName; // 0x2c, name of the type of the receiver
	Toshi::TPString8 m_strGiftType;        // 0x30, type of the given items
	TINT             m_iCount;             // 0x34, number of given items
};

TSTATICASSERT( sizeof( AInteractionGiftEvent ) == 0x38 );
