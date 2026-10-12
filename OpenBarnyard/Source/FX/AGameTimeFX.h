#pragma once
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

// Lighting, fog, sky and rain settings of a moment of the day. The settings
// are loaded from Data\GameTimeFXSettings.trb by AGameTimeFXManager which blends
// between them over the day phases.
class AGameTimeFX
{
public:
	AGameTimeFX();
	AGameTimeFX( const AGameTimeFX& ) = default;
	~AGameTimeFX();

	AGameTimeFX& operator=( const AGameTimeFX& a_rcOther );

	void Apply();
	void Interpolate( const AGameTimeFX& a_rcFrom, const AGameTimeFX& a_rcTo, TFLOAT a_fProgress );

public:
	Toshi::TVector4  m_vecLightDir;            // 0x0
	Toshi::TVector4  m_vecSunFlareDir;         // 0x10, ASunFlareEffect +0x10
	Toshi::TVector4  m_vecShadowColor;         // 0x20
	Toshi::TVector4  m_vecAmbientColor;        // 0x30
	Toshi::TVector4  m_vecFogColor;            // 0x40
	Toshi::TVector4  m_vecSkyTint;             // 0x50, ASkyDome +0x15c
	Toshi::TVector4  m_vecSkySunDir;           // 0x60, ASkyDome +0x17c
	Toshi::TVector4  field7_0x70;              // 0x70
	Toshi::TVector4  field8_0x80;              // 0x80
	Toshi::TMatrix44 m_oSkyColorMatrix;        // 0x90
	Toshi::TMatrix44 m_oSkyLightMatrix;        // 0xd0
	TFLOAT           m_fSkyBlend;              // 0x110
	const TCHAR*     m_szName;                 // 0x114
	TFLOAT           field19_0x118;            // 0x118
	TFLOAT           m_fFogStart;              // 0x11c
	TFLOAT           m_fFogEnd;                // 0x120
	TINT             m_iSkyIndex;              // 0x124
	TFLOAT           m_fShadowAmbientProgress; // 0x128
	TFLOAT           field24_0x12c;            // 0x12c
	TFLOAT           field25_0x130;            // 0x130
	TFLOAT           field26_0x134;            // 0x134, ASunFlareEffect +0x5c
	TFLOAT           m_fSkySunMin;             // 0x138
	TFLOAT           m_fSkySunMax;             // 0x13c
	TFLOAT           m_fStarsOpacity;          // 0x140
	TFLOAT           m_fSkyRotationSpeed;      // 0x144
	TFLOAT           field31_0x148;            // 0x148
	TFLOAT           field32_0x14c;            // 0x14c
	TFLOAT           m_fDarkeningFactor;       // 0x150
	TFLOAT           m_fLightIntensity;        // 0x154
	TFLOAT           m_fBlendStartFraction;    // 0x158, part of the day phase spent before blending to the next one
	TFLOAT           field36_0x15c;            // 0x15c
	TFLOAT           field37_0x160;            // 0x160
	TFLOAT           m_fRainIntensity;         // 0x164
	TFLOAT           m_fRainFallSpeed;         // 0x168
	TFLOAT           m_fRainWindStrength;      // 0x16c
	TFLOAT           m_fRainDropWidth;         // 0x170
	TFLOAT           m_fRainDropLength;        // 0x174
};

TSTATICASSERT( sizeof( AGameTimeFX ) == 0x178 );
