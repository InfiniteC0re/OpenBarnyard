#pragma once
#include "FX/AParticleEffectManager.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// A dotted line made of up to MAX_POINTS copies of a particle effect, each one with
// its own colour. Barn Pool uses it ("poolspot") for the aiming guide of the cue ball.
// Name from the PS2 build (AParticleEmitLine).
// No vtable, size 0x324. Retail has no ctor, ABarnYardPoolMiniGameState's ctor
// clears m_iNumPoints (inlined ctor below).
//-----------------------------------------------------------------------------
class AParticleEmitLine
{
public:
	static constexpr TINT MAX_POINTS = 100;

	struct Point
	{
		AParticleEffectHandle* pHandle; // 0x0
		AParticleEffect*       pEffect; // 0x4, own copy of the effect so the colour can be changed
	};

public:
	AParticleEmitLine() { m_iNumPoints = 0; }

	// Plays MAX_POINTS copies of the effect a_szEffectName, all of them invisible
	void Create( const TCHAR* a_szEffectName );

	// Kills the effects and frees their copies
	void Destroy();

	// Makes every point invisible and removes them from the line
	void Reset();

	// Places points from a_rcFrom towards a_rcTo (a point every 0.2 units) after the
	// ones already placed. a_fOffset (0.0f..1.0f) shifts the points along the line.
	void AddLine( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, TFLOAT a_fOffset );

	// Fades the colour of the points a_iFrom to a_iTo - 1, from a_uiToColour (the
	// point a_iTo - 1) towards a_uiFromColour (the point a_iFrom). Colours are ARGB.
	void SetColours( TINT a_iFrom, TINT a_iTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour );

	TINT GetNumPoints() const { return m_iNumPoints; }

	// Sets colour of the particles of a_pEffect (also used for the arrow of Barn Pool)
	static void SetEffectColour( AParticleEffect* a_pEffect, TUINT32 a_uiColour );

public:
	Point m_aPoints[ MAX_POINTS ]; // 0x0
	TINT  m_iNumPoints;            // 0x320
};

TSTATICASSERT( sizeof( AParticleEmitLine ) == 0x324 );
