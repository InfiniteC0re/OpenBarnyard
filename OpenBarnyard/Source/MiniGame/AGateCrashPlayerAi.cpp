#include "pch.h"
#include "AGateCrashPlayerAi.h"
#include "ARandom.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/ANamedAnimationSet.h"

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( jump_start );

// NOTE: GetClass 00470710, CreateObject 004708d0, CreateObjectInPlace 00470930
TDEFINE_CLASS( AGateCrashPlayerAi );

// $Barnyard: FUNCTION 00470730
AGateCrashPlayerAi::AGateCrashPlayerAi()
{
	for ( TINT i = 0; i < AMiniGameState::MAX_PLAYERS; i++ )
	{
		m_aEntries[ i ].iSlot             = -99;
		m_aEntries[ i ].fReactionDistance = 0.0f;
		m_aEntries[ i ].bKick             = TFALSE;
		m_aEntries[ i ].fKickChance       = 0.5f;
		m_aEntries[ i ].fKickSigma        = 0.15f;
		m_aEntries[ i ].fJumpSigma        = 0.15f;
	}

	m_fJumpStartLength = -1.0f;
}

// $Barnyard: FUNCTION 00470790
// $Barnyard: FUNCTION 00470720
AGateCrashPlayerAi::~AGateCrashPlayerAi()
{
}

// $Barnyard: FUNCTION 004709a0
void AGateCrashPlayerAi::Update( TFLOAT a_fDeltaTime )
{
	if ( m_fJumpStartLength < 0.0f )
	{
		// Length of the jump_start animation of the first player
		AAnimatedUnit*        pUnit         = TSTATICCAST( AAnimatedUnit, m_pGame->aSlots[ 0 ].GetUnit() );
		ANamedAnimationSetRef pAnimationSet = pUnit->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet();

		m_fJumpStartLength = pAnimationSet->GetNamedAnimation( TPS8( jump_start ) )->GetDuration();
	}

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Entry& rEntry = m_aEntries[ i ];

		if ( rEntry.iSlot == -99 )
			continue;

		AGateCrashMinigameState::PlayerSlot& rSlot = m_pGame->aSlots[ rEntry.iSlot ];

		if ( rSlot.eState > AGateCrashMinigameState::PLAYERSTATE_KICKING )
			continue;

		if ( rSlot.eState == AGateCrashMinigameState::PLAYERSTATE_JUMPING )
			rSlot.bJumpHeld = TFALSE;

		TFLOAT fGateSpeed = TMath::Abs( m_pGame->fGateSpeed );

		if ( ( rSlot.eState == AGateCrashMinigameState::PLAYERSTATE_READY && rEntry.fReactionDistance == 0.0f ) ||
		     m_bLastGateDir != ( m_pGame->fGateSpeed > 0.0f ) )
		{
			// Idle and undecided, or the gate has changed direction: decide what to do next
			rEntry.bKick = TFALSE;

			if ( rEntry.iSlot == m_pGame->iServingPlayer )
			{
				// Serve
				rSlot.bKickPressed     = TTRUE;
				rSlot.bDirectionalKick = TFALSE;
			}
			else
			{
				TFLOAT fKickChance = rEntry.fKickChance;

				if ( ( rEntry.fAlwaysJumpSpeed != 0.0f && m_pGame->fGateSpeed >= rEntry.fAlwaysJumpSpeed ) ||
				     ARandom::GetSingleton()->m_oRandom.GetFloat() >= fKickChance )
				{
					// Jump: press the button early enough for jump_start to finish
					rEntry.bKick             = TFALSE;
					rEntry.fReactionDistance = RandomGaussian( 1.0f, fGateSpeed * rEntry.fJumpSigma ) * ( JUMP_REACTION_TIME + m_fJumpStartLength ) * fGateSpeed;
				}
				else
				{
					// Kick
					rEntry.bKick             = TTRUE;
					rEntry.fReactionDistance = RandomGaussian( 1.0f, fGateSpeed * rEntry.fKickSigma ) * KICK_REACTION_ANGLE;
				}
			}
		}

		// Angular distance the gate still has to travel to reach the slot (after this frame)
		TFLOAT fDistance = rSlot.fAngle - m_pGame->fGateAngle;
		TMath::NormaliseAngle( fDistance );

		if ( fDistance < 0.0f )
			fDistance += TMath::TWO_PI;

		if ( m_pGame->fGateSpeed < 0.0f )
			fDistance = TMath::TWO_PI - fDistance;

		fDistance -= fGateSpeed * a_fDeltaTime;

		if ( rEntry.fReactionDistance > fDistance )
		{
			rSlot.bDirectionalKick = TFALSE;

			if ( rEntry.bKick == TTRUE )
				rSlot.bKickPressed = TTRUE;
			else
				rSlot.bJumpPressed = TTRUE;

			rEntry.fReactionDistance = 0.0f;
		}
	}

	m_bLastGateDir = m_pGame->fGateSpeed > 0.0f;
}

// $Barnyard: FUNCTION 004707b0
void AGateCrashPlayerAi::Init( TINT a_iFirstSlot, TINT a_iCount )
{
	AMinigamePlayerAi::Init( a_iFirstSlot, a_iCount );

	for ( TINT i = 0; i < a_iCount; i++ )
		m_aEntries[ i ].iSlot = a_iFirstSlot + i;
}

// $Barnyard: FUNCTION 004707f0
void AGateCrashPlayerAi::SetParams( TINT a_iIndex, TFLOAT a_fKickChance, TFLOAT a_fKickSigma, TFLOAT a_fJumpSigma, TFLOAT a_fAlwaysJumpSpeed )
{
	m_aEntries[ a_iIndex ].fKickChance      = a_fKickChance;
	m_aEntries[ a_iIndex ].fKickSigma       = a_fKickSigma;
	m_aEntries[ a_iIndex ].fJumpSigma       = a_fJumpSigma;
	m_aEntries[ a_iIndex ].fAlwaysJumpSpeed = a_fAlwaysJumpSpeed;
}

// $Barnyard: FUNCTION 00470820
TFLOAT AGateCrashPlayerAi::RandomGaussian( TFLOAT a_fMean, TFLOAT a_fSigma )
{
	// Box-Muller transform
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	// NOTE: TMathInline::Log is logf
	TFLOAT fLog = logf( rRandom.GetFloat() );
	TFLOAT fSin = TMath::Sin( rRandom.GetFloat() * TMath::TWO_PI );

	return TMath::Sqrt( fLog * -2.0f ) * fSin * a_fSigma + a_fMean;
}
