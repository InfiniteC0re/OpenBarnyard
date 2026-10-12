#pragma once
#include <Math/TVector4.h>
#include <Toshi/TRefCounted.h>

#include "Render/AGlowViewport.h"
#include "Units/AUnitProxy.h"

class AUnit;
class AAnimatableObject;

//-----------------------------------------------------------------------------
// Purpose: Helpers of the bike ride (ABikeManager, ABikeSteer, the bike race):
// putting things on the ground, attaching the bike model to the rider and
// moving the hats between the units.
// NOTE: retail free functions (stdcall) at 0x00525960-0x00527090, the original
// file name is unknown.
//-----------------------------------------------------------------------------
namespace ABikeHelpers
{

using SNAPFLAGS = TUINT8;
enum SNAPFLAGS_ : SNAPFLAGS
{
	SNAPFLAGS_NONE     = 0,
	SNAPFLAGS_FORWARD  = BITFLAG( 0 ), // a_fRadius in front
	SNAPFLAGS_BACKWARD = BITFLAG( 1 ), // a_fRadius behind
	SNAPFLAGS_LEFT     = BITFLAG( 2 ), // a_fRadius to the side
	SNAPFLAGS_RIGHT    = BITFLAG( 3 ), // a_fRadius to the other side
	SNAPFLAGS_CENTRE   = BITFLAG( 4 ), // at the position
};

// Puts a_rPosition on the ground (a ray 40 units down from 1 unit above it), y is 0 if there's no ground
TBOOL SnapToGround( Toshi::TVector4& a_rPosition );

// Puts a_rPosition on the ground (a ray a_fLength long from a_fHeight above it) and outputs the normal of the ground (VEC_NEGY if there's none)
TBOOL SnapToGround( Toshi::TVector4& a_rPosition, Toshi::TVector4& a_rOutNormal, TFLOAT a_fHeight, TFLOAT a_fLength );

// Puts a_rPosition on the ground and makes a_rDirection follow its slope
TBOOL AlignToGround( Toshi::TVector4& a_rPosition, Toshi::TVector4& a_rDirection, TFLOAT a_fHeight, TFLOAT a_fLength );

// Puts a_rPosition at the highest ground found at the points of a_eFlags around it
TBOOL SnapToGroundAround( Toshi::TVector4& a_rPosition, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, SNAPFLAGS a_eFlags );

// Pushes a sphere of a_fRadius at a_rPosition out of the collision (on the XZ plane)
void PushOutOfCollision( Toshi::TVector4& a_rPosition, TFLOAT a_fRadius );

// Throws the unit that fell off the bike (AFallingCowSteer) with a_rcVelocity, a_rcPrevPosition is where it was in the last update
void SetupFallingCow( AUnit* a_pUnit, const Toshi::TVector4& a_rcVelocity, const Toshi::TVector4& a_rcDirection, const Toshi::TVector4& a_rcPrevPosition );

// Attaches the model of a_iBikeType to body_loc of a_pUnit
void AttachBikeModel( AUnit* a_pUnit, TINT a_iBikeType );

void RemoveGlowObject( AGlowViewport::GlowObject* a_pGlowObject );

// TRUE if nothing is between a_pBike and a_pUnit (and the bike has both of its wheels on the ground)
TBOOL CanReachBike( AUnit* a_pBike, AUnit* a_pUnit );

// Puts a_pRider and a_pBike where the cow riding the bike (a_pPlayerBike) is
void PlaceBikeAndRider( AUnit* a_pPlayerBike, AUnit* a_pRider, AUnit* a_pBike, TBOOL a_bSnapToGround );
void PlaceBikeAndRider( Toshi::TRef<AUnitProxy> a_pPlayerBike, Toshi::TRef<AUnitProxy> a_pRider, Toshi::TRef<AUnitProxy> a_pBike, TBOOL a_bSnapToGround );

// Moves every object attached to the a_rcBone bone of a_pFrom to the same bone of a_pTo
void MoveAttachedObjects( AAnimatableObject* a_pFrom, AAnimatableObject* a_pTo, const Toshi::TPString8& a_rcBone );

// Creates the glow of the headlight of the bike of a_pUnit
AGlowViewport::GlowObject* CreateHeadlightGlow( AUnit* a_pUnit );

// Moves the hats from a_pFrom to a_pTo
void MoveHeadAttachments( AUnit* a_pFrom, AUnit* a_pTo );

} // namespace ABikeHelpers
