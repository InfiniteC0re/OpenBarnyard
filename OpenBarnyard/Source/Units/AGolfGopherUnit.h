#pragma once
#include "AAnimatedUnit.h"

#include <Toshi/TEvent.h>
#include <Math/TVector4.h>

#include "Animation/AAnimationBreakpointCallback.h"

//-----------------------------------------------------------------------------
// The gopher of the Gopher Golf minigame (unit type "golfgopher"), one per
// player. It tunnels underground (its steer is an AGopherMoundSteer) and leaves
// a trail of sods behind: every 0.4 units moved, the next of the 20 trail
// objects plays "l_tunnel"/"r_tunnel" (alternating) and fades out. When the
// gopher model reaches one of the surfacing breakpoints, a ring of 5 mound
// objects plays "surface" around it; the dive breakpoints (and the end of the
// surfacing, see Think) make the mound play "submerge" and fade out.
//
// TClass 0077e7b8 (parent AAnimatedUnit, size 0x13c), vtables 007184f8,
// 007184e8 (AGoalCollisionUpdateInterface).
//-----------------------------------------------------------------------------
class AGolfGopherUnit
    : public AAnimatedUnit
{
public:
	TDECLARE_CLASS( AGolfGopherUnit, AAnimatedUnit );

	static constexpr TINT NUM_TRAIL_OBJECTS = 20;
	static constexpr TINT NUM_MOUND_OBJECTS = 5;

	// A listener connected to an animation breakpoint emitter of the gopher model
	struct AnimListener
	    : public Toshi::TDList<AnimListener>::TNode
	{
		AnimListener( Toshi::TGenericEmitter& a_rEmitter, void* a_pCaller, Toshi::TGenericListener::t_Callback a_fnCallback, TINT a_iPriority );

		// $Barnyard: FUNCTION 00417d80
		~AnimListener() { oListener.Disconnect(); }

		Toshi::TGenericListener oListener; // 0x08
		Toshi::TGenericEmitter* pEmitter;  // 0x1c
	};

	// Listeners that all share a caller and a callback
	struct AnimListeners
	{
		void*                               pCaller;    // 0x00, the gopher
		Toshi::TGenericListener::t_Callback fnCallback; // 0x04
		Toshi::TDList<AnimListener>         oList;      // 0x08

		// $Barnyard: FUNCTION 00418a50
		~AnimListeners() { FreeAll(); }

		// Connects a new listener to a_rEmitter
		AnimListener* Add( Toshi::TGenericEmitter& a_rEmitter, TINT a_iPriority );

		// Disconnects and frees every listener
		void FreeAll();
	};

public:
	AGolfGopherUnit();
	~AGolfGopherUnit();

	//-----------------------------------------------------------------------------
	// AUnit
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( AUnitTransform* a_pTransform ) override; // +0x58
	virtual void  OnDestroy() override;                            // +0x60
	virtual TBOOL Think( TFLOAT a_fDeltaTime ) override;           // +0x6c

	// Restarts the trail at a_rcPosition and forgets the mound (called by the
	// golf state after it teleports the gopher)
	void ResetTrail( const Toshi::TVector4& a_rcPosition );

	// Plays "surface" on the mound objects around the gopher
	void Surface();

	// Plays "submerge" on the mound objects
	void Submerge();

private:
	static TBOOL __stdcall OnSurfaceEventCallback( AGolfGopherUnit* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData );
	static TBOOL __stdcall OnSubmergeEventCallback( AGolfGopherUnit* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData );

private:
	AnimListeners      m_oSurfaceListeners;            // 0x98, inground_surface, aboveground_surface, surface, aboveground_dive, victory_surface, lose_surface
	AnimListeners      m_oSubmergeListeners;           // 0xa8, inground_dive, strike
	AAnimatableObject* m_apTrail[ NUM_TRAIL_OBJECTS ]; // 0xb8, random "sod_1".."sod_3"
	AAnimatableObject* m_apMound[ NUM_MOUND_OBJECTS ]; // 0x108, random "sod_1".."sod_3"
	Toshi::TVector4    m_vTrailPosition;               // 0x11c, where the last trail object was placed
	TINT               m_iNextTrail;                   // 0x12c, index in m_apTrail
	TBOOL              m_bTrailLeft;                   // 0x130, the last trail object played "l_tunnel"
	TBOOL              m_bSurfaced;                    // 0x131, the mound is shown
	TINT               m_iMoveEffect;                  // 0x134, "gopher_move"
	TINT               m_iMoveChunksEffect;            // 0x138, "gopher_move_chunks"
};

TSTATICASSERT( sizeof( AGolfGopherUnit ) == 0x13c );
