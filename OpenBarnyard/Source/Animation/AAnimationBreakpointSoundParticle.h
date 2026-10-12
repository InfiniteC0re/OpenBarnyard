#pragma once
#include "AAnimationBreakpoint.h"
#include "ASoundBreakpoint.h"

#include <Toshi/TPString8.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Plays a sound and a particle effect at a bone of the object. Created for the
// sound breakpoints of the animations (ANamedAnimationSet::AddSoundBreakpoint)
// and set up by SetSoundBreakpoint, Create only reads "Time".
// Footsteps pick the cue's track and the effect from the surface below the bone.
//-----------------------------------------------------------------------------
class AAnimationBreakpointSoundParticle
    : public AAnimationBreakpoint
{
public:
	TDECLARE_CLASS( AAnimationBreakpointSoundParticle, AAnimationBreakpoint );

	// Surface below a footstep (see GetFootstepSurface), used as the track of the cue
	using SURFACE = TINT;
	enum SURFACE_ : SURFACE
	{
		SURFACE_DEFAULT = -1, // not a footstep
		SURFACE_GRASS   = 0,  // TOSHICGROUP_GRASS
		SURFACE_DIRT    = 1,  // TOSHICGROUP_DIRT
		SURFACE_TIMBER  = 2,  // TOSHICGROUP_TIMBER, also the barn
		SURFACE_WATER   = 3,  // TOSHICGROUP_WATER, TOSHICGROUP_DEEPWATER
		SURFACE_STONE   = 4,  // TOSHICGROUP_STONE
		SURFACE_TIN     = 5,  // TOSHICGROUP_TIN
		SURFACE_SAND    = 6,  // TOSHICGROUP_SAND
		SURFACE_MUD     = 7,  // TOSHICGROUP_MUD
		SURFACE_HAY     = 8,  // TOSHICGROUP_HAY
		SURFACE_NUMOF   = 9,
		SURFACE_NONE    = 10, // nothing below, the breakpoint does nothing
	};

public:
	AAnimationBreakpointSoundParticle();
	~AAnimationBreakpointSoundParticle();

	//-----------------------------------------------------------------------------
	// AAnimationBreakpoint
	//-----------------------------------------------------------------------------
	virtual void Execute( AAnimatableObject* a_pObject, TFLOAT a_fTime ) OVERRIDE; // +0x14

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL SetSoundBreakpoint( const ASoundBreakpoint* a_pSoundBreakpoint ); // +0x18

private:
	// Returns surface below a_rTransform's translation and moves the translation onto it
	static SURFACE GetFootstepSurface( Toshi::TMatrix44& a_rTransform );

private:
	ASoundId         m_eSoundId;                                         // 0x08
	TFLOAT           m_fVolume;                                          // 0x0c, always 1.0f
	TBOOL            m_bFootstep;                                        // 0x10, ASoundBreakpoint::FLAGS_FOOTSTEP
	Toshi::TPString8 m_strBoneName;                                      // 0x14
	TBOOL            m_bEffectDown;                                      // 0x18, ASoundBreakpoint::FLAGS_EFFECT_DOWN
	TINT             m_aEffects[ ASoundBreakpointEffects::NUM_EFFECTS ]; // 0x1c, [ 1 + surface ], see ASoundBreakpointEffects
	TINT             m_iWaterRunEffect1;                                 // 0x44, "waterrun", footsteps while the weather (0x00783ecc) is in state 3 or 5
	TINT             m_iWaterRunEffect2;                                 // 0x48, "waterrun", footsteps while the weather is in state 2
	TBOOL            m_bVOStream;                                        // 0x4c, ASoundBreakpoint::FLAGS_VO_STREAM
};

TSTATICASSERT( sizeof( AAnimationBreakpointSoundParticle ) == 0x50 );
