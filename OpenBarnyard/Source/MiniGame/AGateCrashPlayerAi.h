#pragma once
#include "AMinigamePlayerAi.h"
#include "AGateCrashMinigameState.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Computer players of Mud Jumpers. For every AI slot it picks jump or kick
// when the gate changes direction (or the slot is idle and undecided) and
// presses the button once the gate is close enough (a Gaussian-randomised
// reaction distance).
// TClass 007827a4, vtable 0071db58.
// NOTE: the retail TClass parent is TObject, not AMinigamePlayerAi.
//-----------------------------------------------------------------------------
class AGateCrashPlayerAi
    : public AMinigamePlayerAi
{
public:
	TDECLARE_CLASS( AGateCrashPlayerAi, Toshi::TObject );

	static constexpr TFLOAT KICK_REACTION_ANGLE = Toshi::TMath::PI / 12.0f; // 0x00782798, kick window of AGateCrashMinigameState
	static constexpr TFLOAT HIT_ANGLE           = Toshi::TMath::PI / 36.0f; // 0x0078279c
	static constexpr TFLOAT JUMP_REACTION_TIME  = 0.2449f;                  // 0x007827a0, added to the jump_start animation length

	// NOTE: muddata.trb AIData p0..p3 map to fKickChance, fKickSigma, fJumpSigma and fAlwaysJumpSpeed.
	struct Entry
	{
		TINT   iSlot;             // 0x00, -99 when unused
		TFLOAT fReactionDistance; // 0x04, angular distance left at which the button is pressed, 0 = undecided
		TFLOAT fKickChance;       // 0x08, probability of kicking instead of jumping (AIData p0)
		TFLOAT fKickSigma;        // 0x0c, AIData p1
		TFLOAT fJumpSigma;        // 0x10, AIData p2
		TFLOAT fAlwaysJumpSpeed;  // 0x14, AIData p3, always jump from this gate speed on (0 = never)
		TBOOL  bKick;             // 0x18
	};

public:
	AGateCrashPlayerAi();
	~AGateCrashPlayerAi();

	//-----------------------------------------------------------------------------
	// AMinigamePlayerAi
	//-----------------------------------------------------------------------------
	virtual void Update( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void Init( TINT a_iFirstSlot, TINT a_iCount ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Applies one muddata.trb AIData entry.
	void SetParams( TINT a_iIndex, TFLOAT a_fKickChance, TFLOAT a_fKickSigma, TFLOAT a_fJumpSigma, TFLOAT a_fAlwaysJumpSpeed ); // 004707f0

	// Box-Muller normal distribution sample using ARandom.
	TFLOAT RandomGaussian( TFLOAT a_fMean, TFLOAT a_fSigma ); // 00470820

	void SetGameData( AGateCrashMinigameState::GameData* a_pGame ) { m_pGame = a_pGame; }

public:
	Entry                              m_aEntries[ AMiniGameState::MAX_PLAYERS ]; // 0x0c
	AGateCrashMinigameState::GameData* m_pGame;                                   // 0x7c, AGateCrashMinigameState + 0xbac
	TBOOL                              m_bLastGateDir;                            // 0x80, gate speed was > 0 last frame
	TFLOAT                             m_fJumpStartLength;                        // 0x84, -1 until fetched from jump_start
};

TSTATICASSERT( sizeof( AGateCrashPlayerAi::Entry ) == 0x1c );
TSTATICASSERT( sizeof( AGateCrashPlayerAi ) == 0x88 );
