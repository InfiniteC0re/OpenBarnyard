#pragma once
#include "ACharacterUnit.h"

#include <Math/TVector4.h>

class AInteriorNetwork;
class AInteriorSteer;
class AThrowItemSteer;
class AAnimatableObject;

//-----------------------------------------------------------------------------
// A raccoon of the Sharp Squirter minigame (unit type "sharp_racoon", spawned
// at the "racoon" goals by APrecisionSquirt). It holds a slingshot
// ("precision_slingshot") and a ball ("precision_ball") it can shoot at the
// player. Once milked it runs off to the "respawn" goal farthest from the
// player and fades out.
// NOTE: nothing in the game sets STATE_SLINGSHOT, STATE_CHASE or STATE_RETURN,
// the raccoons only idle until they're milked.
// TClass 007091f0 (parent ACharacterUnit, size 0x19c), vtables 00721a90, 00721880.
//-----------------------------------------------------------------------------
class APrecisionRacoon
    : public ACharacterUnit
{
public:
	TDECLARE_CLASS( APrecisionRacoon, ACharacterUnit );

	static constexpr TINT MAX_RESPAWNS = 10;

	enum STATE_ : TINT
	{
		STATE_IDLE,      // 0
		STATE_MILKED,    // 1, "yuk"
		STATE_FLEE,      // 2, runs to the farthest respawn goal
		STATE_FADE,      // 3, fades out
		STATE_SLINGSHOT, // 4, faces the player and shoots the ball at him
		STATE_HIDDEN,    // 5
		STATE_CHASE,     // 6, runs after the player
		STATE_RETURN,    // 7, runs back to its spawn position
	};

	enum SLINGSHOT_ : TINT
	{
		SLINGSHOT_DRAW, // 0, puts the ball in the slingshot
		SLINGSHOT_AIM,  // 1
		SLINGSHOT_FIRE, // 2
		SLINGSHOT_FLY,  // 3, the ball is on its way
	};

public:
	APrecisionRacoon();
	~APrecisionRacoon();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) OVERRIDE; // +0x58
	virtual void  OnDestroy() OVERRIDE;                            // +0x60
	virtual void  UpdateAI( TFLOAT a_fDeltaTime ) OVERRIDE;        // +0x68

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	void SetState( STATE_ a_eState );

	// Collects the positions of the "respawn" goals of a_pNetwork
	void SetNetwork( AInteriorNetwork* a_pNetwork );

	// Sets the player the raccoon shoots at
	void SetPlayer( AUnit* a_pPlayer );

	// Replaces a_rPosition with the respawn position farthest from a_rcFrom (if it's farther). TFALSE if there are none.
	TBOOL FindFarthestRespawn( const Toshi::TVector4& a_rcFrom, Toshi::TVector4& a_rPosition );

	// TTRUE: the raccoon has been hit by the milk. TFALSE: puts it back at its spawn position.
	void SetMilked( TBOOL a_bMilked );

	// Draws, aims and fires the slingshot
	void UpdateSlingshot( TFLOAT a_fDeltaTime );

	TBOOL IsMilked() const { return m_bMilked; }

public:
	TBOOL              m_bMilked;                   // 0xa0
	TBOOL              m_bStateChanged;             // 0xa1
	TBOOL              m_bHitPlayer;                // 0xa2, the ball has hit the player
	TBOOL              m_bBallHit;                  // 0xa3, the ball has hit something
	TFLOAT             m_fOpacity;                  // 0xa4
	TFLOAT             m_fAimTime;                  // 0xa8
	TFLOAT             m_fAimTimer;                 // 0xac
	Toshi::TVector4    m_vBallPosition;             // 0xb0, position of the ball in the last frame
	AInteriorNetwork*  m_pNetwork;                  // 0xc0
	AUnit*             m_pPlayer;                   // 0xc4
	AInteriorSteer*    m_pSteer;                    // 0xc8
	AThrowItemSteer*   m_pBallSteer;                // 0xcc
	AAnimatableObject* m_pSlingshot;                // 0xd0
	AUnit*             m_pBall;                     // 0xd4
	STATE_             m_eState;                    // 0xd8
	SLINGSHOT_         m_eSlingshot;                // 0xdc
	TINT               m_iNumRespawns;              // 0xe0
	Toshi::TVector4    m_aRespawns[ MAX_RESPAWNS ]; // 0xe4
	Toshi::TVector4    m_vSpawnPosition;            // 0x184
	TINT               m_iSpawnArea;                // 0x194
	TINT               m_iMapIcon;                  // 0x198
};

TSTATICASSERT( sizeof( APrecisionRacoon ) == 0x19c );
