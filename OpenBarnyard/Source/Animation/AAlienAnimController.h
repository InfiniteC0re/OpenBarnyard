#pragma once
#include "ACharacterAnimController.h"

//-----------------------------------------------------------------------------
// Purpose: ACharacterAnimController of the player characters steered by
// AAlienSteer. Adds the milk squirting movement: the milk_run animation of the
// direction the character moves relative to where it faces, and the combat rolls.
//-----------------------------------------------------------------------------
class AAlienAnimController
    : public ACharacterAnimController
{
public:
	TDECLARE_CLASS( AAlienAnimController, ACharacterAnimController );

	static constexpr TINT NUM_DIRECTIONS = 8;

public:
	AAlienAnimController();
	~AAlienAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual void OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void SetAnimEnabled( TBOOL a_bValue );

	// Returns which of the 8 directions (N, NE, E, ...) a_rcMovement points to relative to a_rcFacing
	static TINT GetDirectionIndex( const Toshi::TVector4& a_rcFacing, const Toshi::TVector4& a_rcMovement );

private:
	// milk_run animation of each direction (filled by the constructor)
	static const Toshi::TPString8* ms_apRunAnims[ NUM_DIRECTIONS ];

private:
	const Toshi::TPString8* m_pRollAnim;    // 0x24, milk_combatroll_l or milk_combatroll_r
	TBOOL                   m_bRolling;     // 0x28
	TBOOL                   m_bWasStrafing; // 0x29, AAlienSteer::IsStrafingActive last frame
	TBOOL                   m_bAnimEnabled; // 0x2a
};
