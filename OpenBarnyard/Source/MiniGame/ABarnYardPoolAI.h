#pragma once
#include <Math/TVector4.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TRefCounted.h>

class AUnitProxy;
class AUnitPoolBall;

//-----------------------------------------------------------------------------
// Shot planner of the computer players in Barn Pool. Embedded in
// ABarnYardPoolMiniGameState at +0x2658 (ctor 004ce6e0, dtor 004ce400).
// GetShot tries three strategies in order:
//  1. FindPotShot: pot a ball of the target group into a pocket.
//  2. FindHitShot: just hit a ball of the target group (no pot).
//  3. GetFallbackShot: shoot the cue ball straight away from the black.
// and then adds a random aim error that depends on the difficulty.
// NOTE: the name is invented, no TOSHI build leaks the real one.
//-----------------------------------------------------------------------------
class ABarnYardPoolAI
{
public:
	// Pot candidate: a ball and one of the approach points around a pocket.
	struct PotEntry
	{
		Toshi::TVector4 vecBall;   // 0x00, ball position
		Toshi::TVector4 vecTarget; // 0x10, approach point around the pocket
	};

	// Shot candidate checked against the table and other balls.
	struct ShotEntry
	{
		Toshi::TVector4 vecStart;  // 0x00, cue ball position
		Toshi::TVector4 vecGhost;  // 0x10, where the cue ball has to be when it hits the object ball
		Toshi::TVector4 vecTarget; // 0x20, where the object ball goes (pocket approach point, or the ball itself for a hit shot)
		Toshi::TVector4 vecBall;   // 0x30, object ball position (not set for a hit shot)
	};

	static constexpr TINT MAX_NUM_POT_ENTRIES  = 3200;
	static constexpr TINT MAX_NUM_SHOT_ENTRIES = 300;

	// Ball type value meaning "either of the two groups" (open table)
	static constexpr TINT BALLTYPE_ANY_GROUP = 4;

	// Ball state of a pocketed ball
	static constexpr TINT BALLSTATE_POCKETED = 3;

public:
	ABarnYardPoolAI();
	~ABarnYardPoolAI();

	void Reset();

	// Returns the cue ball velocity of the computer's next shot (direction scaled by power).
	// a_bNoAimError skips the difficulty-based random aim error.
	Toshi::TVector4 GetShot( TBOOL a_bNoAimError );

	// Ball in hand: moves the cue ball to a spot inside the area in front of a_rcCenter from which
	// a hit shot exists, and limits the next shot to the a_vForward half.
	void PlaceCueBall( Toshi::TVector4 a_vCenter, TFLOAT a_fCueBallSize, TFLOAT a_fAreaRadius, Toshi::TVector4 a_vForward );

	// Returns TTRUE if a_pBall overlaps any other ball.
	TBOOL IsBallOverlapping( AUnitPoolBall* a_pBall );

	// Clamps a_vPoint into the ball-in-hand area (radius a_fRadius around a_vCenter, shrunk to the sides,
	// only the half behind a_vForward). Returns TTRUE if the point was already inside.
	// NOTE: retail places this function next to the state's (004d1ca0); the state (004d2180) uses it as well.
	static TBOOL ClampToBallInHandArea( Toshi::TVector4 a_vPoint, Toshi::TVector4 a_vCenter, TFLOAT a_fRadius, Toshi::TVector4 a_vForward, Toshi::TVector4* a_pOutPoint );

	// $Barnyard: FUNCTION 004ce0b0
	void SetBalls( TINT a_iNumBalls, const Toshi::TRef<AUnitProxy>* a_pBalls, TFLOAT a_fBallRadius )
	{
		m_iNumBalls   = a_iNumBalls;
		m_pBalls      = a_pBalls;
		m_fBallRadius = a_fBallRadius;
	}

	// $Barnyard: FUNCTION 004ce0d0
	void SetCueAndBlackBall( AUnitPoolBall* a_pCueBall, AUnitPoolBall* a_pBlackBall )
	{
		m_pCueBall   = a_pCueBall;
		m_pBlackBall = a_pBlackBall;
	}

	// $Barnyard: FUNCTION 004ce0f0
	void SetPockets( TINT a_iNumPockets, const Toshi::TVector4* a_pPockets, TFLOAT a_fPocketRadius )
	{
		m_iNumPockets   = a_iNumPockets;
		m_pPockets      = a_pPockets;
		m_fPocketRadius = a_fPocketRadius;
	}

	// $Barnyard: FUNCTION 004ce120
	void SetForward( Toshi::TVector4 a_vForward ) { m_vecForward = a_vForward; }

	// Folded setter 00566c60
	void SetTargetBallType( TINT a_eBallType ) { m_eTargetBallType = a_eBallType; }

	// Folded setter 004ce110
	void SetDifficulty( TINT a_iDifficulty ) { m_iDifficulty = a_iDifficulty; }

	// Folded setter 0043ce10 (T2GUIElement::SetHeight)
	void SetMaxShotPower( TFLOAT a_fMaxShotPower ) { m_fMaxShotPower = a_fMaxShotPower; }

	// Cue ball position moved two ball radii along the last planned shot
	const Toshi::TVector4& GetAimPoint() const { return m_vecAimPoint; }

private:
	Toshi::TVector4 FindPotShot();
	Toshi::TVector4 FindHitShot();
	Toshi::TVector4 GetFallbackShot();

	void AddApproachPoints( TINT a_eType, Toshi::TVector4 a_vStart, TFLOAT a_fUnused, Toshi::TVector4 a_vCenter, TFLOAT a_fRadius );

	AUnitPoolBall* GetBall( TINT a_iIndex ) const;

	// Removes shot candidates that hit another ball, the table, need too thin a cut or go backwards on ball in hand.
	// a_bPotShot selects which ShotEntry position is the object ball (vecBall for pot shots, vecTarget for hit shots).
	// NOTE: retail has these loops inlined in both FindPotShot and FindHitShot.
	void FilterShotEntries( TBOOL a_bPotShot, TFLOAT a_fBallSweepEpsilon );

	// Gets the shot candidate with the shortest ghost to target distance. Returns TFALSE if there is none.
	TBOOL FindBestShotEntry( ShotEntry& a_rOutEntry );

private:
	Toshi::TVector4                                  m_vecAimPoint;        // 0x00
	TINT                                             m_iNumBalls;          // 0x10, set but unused (16 is hardcoded)
	TINT                                             m_iNumPockets;        // 0x14
	TINT                                             m_eTargetBallType;    // 0x18, AUnitPoolBall::GetBallType value, BALLTYPE_ANY_GROUP for both groups
	TINT                                             m_iDifficulty;        // 0x1c, 0..4, aim error +-5, 3, 2, 1, 0 degrees
	TINT                                             m_iNumApproachPoints; // 0x20, points around a pocket or a ball
	TBOOL                                            m_bBallInHand;        // 0x24, the shot must go along m_vecForward
	Toshi::TVector4                                  m_vecForward;         // 0x28
	TFLOAT                                           m_fPocketRadius;      // 0x38
	TFLOAT                                           m_fBallRadius;        // 0x3c
	TFLOAT                                           m_fMaxShotPower;      // 0x40
	const Toshi::TRef<AUnitProxy>*                   m_pBalls;             // 0x44, 16 balls
	AUnitPoolBall*                                   m_pCueBall;           // 0x48
	AUnitPoolBall*                                   m_pBlackBall;         // 0x4c
	const Toshi::TVector4*                           m_pPockets;           // 0x50
	Toshi::T2Vector<PotEntry, MAX_NUM_POT_ENTRIES>   m_vecPotEntries;      // 0x54
	Toshi::T2Vector<ShotEntry, MAX_NUM_SHOT_ENTRIES> m_vecShotEntries;     // 0x19058
};
