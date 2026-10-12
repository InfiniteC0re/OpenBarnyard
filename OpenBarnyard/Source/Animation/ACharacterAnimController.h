#pragma once
#include "AAnimController.h"
#include "ANamedAnimationSet.h"

#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Purpose: Animation controller of the walking characters. Places the model at
// the owner unit's steer every frame and plays the idle/walk/run/sprint loops
// matching the steer's movement. Animations played through the animatable
// object (scripts, interactions) take over until a movement animation plays again.
//-----------------------------------------------------------------------------
class ACharacterAnimController
    : public AAnimController
{
public:
	TDECLARE_CLASS( ACharacterAnimController, AAnimController );

public:
	ACharacterAnimController();
	~ACharacterAnimController();

	//-----------------------------------------------------------------------------
	// AAnimController
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE;                                               // +0x0c
	virtual void  OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                          // +0x10
	virtual TBOOL Unknown3( void* ) OVERRIDE;                                        // +0x14
	virtual TBOOL IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation ) OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Returns TTRUE if one of the animations the controller handles itself is playing
	virtual TBOOL IsMovementAnimPlaying(); // +0x1c

	// Places the model at the owner's steer
	virtual void UpdateTransform( TFLOAT a_fDeltaTime ); // +0x20

	// $Barnyard: FUNCTION 004c8340
	ANamedAnimationSetRef GetAnimationSet() const { return m_pAnimationSet; }

	// Sets the animation set called *a_pName (or the default one if a_pName is TNULL)
	void SetAnimationSet( const Toshi::TPString8* a_pName );

	// Plays the movement animation matching the steer's movement type
	void UpdateMovementAnimation( TFLOAT a_fDeltaTime );

	// Pre-render callback set for the UserRender types: renders a single mesh of the LOD each frame
	static void UserRenderCallback( Toshi::TModelInstance* a_pInstance );

protected:
	TBOOL                 m_bPlayingCustomAnim; // 0x1c
	ANamedAnimationSetRef m_pAnimationSet;      // 0x20
};
