#pragma once
#include <Toshi/T2DList.h>

class AAnimatableObject;
class AAnimControllerType;
class AToshiAnimationInterface;
class ANamedAnimation;

//-----------------------------------------------------------------------------
// Purpose: Base class of the animation controllers. An AAnimatableObjectType
// creates one AAnimControllerType for each "AnimControllerType" block of its
// properties, and every AAnimatableObject of that type creates the controllers
// they describe (AAnimControllerType::GetControllerClass). The controllers are
// updated after the animations of the object (AAnimatableObject::Update), place
// the model at the owner unit and pick the movement animations.
//-----------------------------------------------------------------------------
class AAnimController
    : public Toshi::TObject
    , public Toshi::T2DList<AAnimController>::Node
{
public:
	TDECLARE_CLASS( AAnimController, Toshi::TObject );

public:
	AAnimController();
	virtual ~AAnimController();

	// RETURNS_TRUE 0046cde0
	virtual TBOOL OnCreate() { return TTRUE; } // +0x0c
	// DOES_NOTHING 00628f40
	virtual void OnUpdate( TFLOAT a_fDeltaTime ) {} // +0x10
	// RETURNS_TRUE 00413920
	virtual TBOOL Unknown3( void* ) { return TTRUE; } // +0x14
	// Called before an animation is started by AAnimatableObject. Returning TFALSE queues the animation
	// RETURNS_TRUE 00413920
	virtual TBOOL IsAnimationAllowed( ANamedAnimation* a_pNamedAnimation ) { return TTRUE; } // +0x18

	TBOOL Create( void* a_pOwner, AAnimatableObject* a_pAnimatableObject, AAnimControllerType* a_pControllerType );

	TBOOL IsEnabled() const { return !IsDisabled(); }

	// $Barnyard: FUNCTION 00580380
	TBOOL IsDisabled() const { return m_bDisabled; }

	// $Barnyard: FUNCTION 00580390
	void SetDisabled( TBOOL a_bDisabled ) { m_bDisabled = a_bDisabled; }

	AToshiAnimationInterface& GetAnimInterface();

protected:
	void*                m_pOwner;            // 0x0c, owner (AUnit)
	AAnimatableObject*   m_pAnimatableObject; // 0x10
	AAnimControllerType* m_pControllerType;   // 0x14
	TBOOL                m_bDisabled;         // 0x18
};
