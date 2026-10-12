#pragma once
#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// Base of the per-minigame computer player logic. AMiniGameState owns one
// (m_pPlayerAi), resets it on insertion and updates it every unpaused frame.
// The AI drives player slots [ m_iEndSlot - m_iNumPlayers, m_iEndSlot ).
// TClass 00782730, vtable 0071d8b4.
//-----------------------------------------------------------------------------
class AMinigamePlayerAi
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AMinigamePlayerAi, Toshi::TObject );

public:
	AMinigamePlayerAi();
	virtual ~AMinigamePlayerAi();

	// Called by AMiniGameState::OnInsertion.
	virtual void Reset() {} // DOES_NOTHING, +0x0c

	// Called every frame by AMiniGameState::OnUpdate while the game isn't paused.
	virtual void Update( TFLOAT a_fDeltaTime ) {} // DOES_NOTHING, +0x10

	// Hands the AI the slots it controls.
	virtual void Init( TINT a_iFirstSlot, TINT a_iCount ); // +0x14

protected:
	TINT m_iEndSlot;    // 0x04, a_iFirstSlot + a_iCount
	TINT m_iNumPlayers; // 0x08, a_iCount
};

TSTATICASSERT( sizeof( AMinigamePlayerAi ) == 0xc );
