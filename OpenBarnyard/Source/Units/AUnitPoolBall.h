#pragma once
#include "AAnimatedUnit.h"

class APoolBallSteer;

//-----------------------------------------------------------------------------
// Purpose: A ball of the Barn Pool minigame. Moved by an APoolBallSteer (the
// state changes the steer of the unit and calls SetupSteer), the model follows
// the position of the steer in UpdateAI.
// TClass 0x0077e8e8 (version 1.0, size 0xb4, runtime creatable), vtable 0x00718a68,
// secondary (AGoalCollisionUpdateInterface) vtable 0x00718a5c.
// NOTE: original names of the members and own methods are unknown.
//-----------------------------------------------------------------------------
class AUnitPoolBall : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AUnitPoolBall, AAnimatedUnit );

	// The type of the ball (m_iBallType), set by the state.
	// NOTE: the state uses 0 for the cue ball, 1 for the black ball and 2-3 for the
	// two groups (ABarnYardPoolMiniGameState::BALLTYPE), BALLTYPE_NONE is the only one used here.
	static constexpr TINT BALLTYPE_NONE = 4;

public:
	AUnitPoolBall();
	~AUnitPoolBall();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) override;        // +0x68
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Caches m_pSteer, which has to be an APoolBallSteer by then.
	void SetupSteer();

	// Position of the steer.
	Toshi::TVector4 GetPosition();

	// Moves the steer to a_rcPosition (and stops it).
	void SetPosition( const Toshi::TVector4& a_rcPosition );

	// Radius of the ball (APoolBallSteer::GetRadius).
	TFLOAT GetRadius();

	// TTRUE if the ball doesn't touch any other pool ball.
	TBOOL IsPlacementValid();

	// APoolBallSteer::State of the steer.
	TINT GetSteerState();

	// Type of the first ball this ball hit since the last call (BALLTYPE_NONE if none), and forgets it.
	TINT GetFirstHitBallType();

	// $Barnyard: FUNCTION 0041a370
	void SetStartPosition( const Toshi::TVector4& a_rcPosition ) { m_vecStartPosition = a_rcPosition; }

	// $Barnyard: FUNCTION 0041a3a0
	Toshi::TVector4 GetStartPosition() { return m_vecStartPosition; }

	// $Barnyard: FUNCTION 0041a3e0
	void SetBallType( TINT a_iBallType ) { m_iBallType = a_iBallType; }

	// $Barnyard: FUNCTION 0041a3f0
	TINT GetBallType() { return m_iBallType; }

	// $Barnyard: FUNCTION 0041a400
	void SetPotted( TBOOL a_bPotted ) { m_bPotted = a_bPotted; }

	// NOTE: folded with ASkinShaderHAL::IsHighEndSkinning (0041a410) in retail
	TBOOL IsPotted() { return m_bPotted; }

	APoolBallSteer* GetPoolSteer() const { return m_pPoolSteer; }

private:
	Toshi::TVector4 m_vecStartPosition; // 0x98, set by the state when it racks the balls
	TINT            m_iBallType;        // 0xa8, BALLTYPE_NONE by default
	TBOOL           m_bPotted;          // 0xac, set by the state once it has counted the potted ball
	APoolBallSteer* m_pPoolSteer;       // 0xb0, m_pSteer, see SetupSteer
};

TSTATICASSERT( sizeof( AUnitPoolBall ) == 0xb4 );
