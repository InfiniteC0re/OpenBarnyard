#pragma once
#include "Sound/ASound.h"

// Bone and particle effects of a sound breakpoint
struct ASoundBreakpointEffects
{
	static constexpr TINT NUM_EFFECTS = 10;

	const TCHAR* pszBoneName;                        // 0x0, the effects and the sound are placed at this bone (TNULL: the model)
	const TCHAR* apszParticleEffects[ NUM_EFFECTS ]; // 0x4, [ 0 ]: default effect, [ 1 + surface ]: footstep effect on the surface
};

struct ASoundBreakpoint
{
	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE        = 0,
		FLAGS_FOOTSTEP    = BITFLAG( 0 ), // picks the cue's track and the effect from the surface below the bone
		FLAGS_VO_STREAM   = BITFLAG( 1 ), // eSoundId is a sub stream of the VO stream
		FLAGS_EFFECT_DOWN = BITFLAG( 2 ), // effects face down (TVector4::VEC_NEGY) instead of the bone's Y axis
	};

	TCHAR*                   pszModelName;     // 0x00
	TCHAR*                   pszAnimationName; // 0x04, exported name of the animation
	TFLOAT                   fTime;            // 0x08
	ASoundId                 eSoundId;         // 0x0c
	FLAGS                    eFlags;           // 0x10
	ASoundBreakpointEffects* pEffects;         // 0x14
};

TSTATICASSERT( sizeof( ASoundBreakpoint ) == 0x18 );

// Stored in TRB files
struct ASoundBreakpointsTable
{
	TUINT32           uiCount;
	ASoundBreakpoint* pBreakpoints;
};
