#pragma once
#include "Render/ASysShader/ASysMesh.h"
#include "Render/ASysShader/ASysMaterial.h"
#include "Assets/AMaterialLibrary.h"

#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Draws the sun (with its lens flare) and the moon as screen space quads.
// Created by ARenderer::OnCreate and drawn by ARenderer::RenderMainScene.
// AGameTimeFX::Apply sets the directions and intensities, in the main game the
// sun direction follows the time of the day instead.
// NOTE: not a TObject, the retail singleton pointer is at 0x0079b328.
//-----------------------------------------------------------------------------
class ASunFlareEffect
    : public Toshi::TSingleton<ASunFlareEffect>
{
public:
	ASunFlareEffect();
	~ASunFlareEffect();

	void Render();

	void SetSunDirection( const Toshi::TVector4& a_rcDir ) { m_vecSunDir = a_rcDir; }
	void SetMoonDirection( const Toshi::TVector4& a_rcDir ) { m_vecMoonDir = a_rcDir; }
	void SetSunDirections( const Toshi::TVector4& a_rcSunrise, const Toshi::TVector4& a_rcNoon )
	{
		m_vecSunriseDir = a_rcSunrise;
		m_vecNoonDir    = a_rcNoon;
	}

	void SetSunIntensity( TFLOAT a_fIntensity ) { m_fSunIntensity = a_fIntensity; }
	void SetMoonIntensity( TFLOAT a_fIntensity ) { m_fMoonIntensity = a_fIntensity; }
	void SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

	// Makes the sun follow this time instead of the time of AGameTimeManager
	// $Barnyard: FUNCTION 0043fb30
	void SetTime( TINT a_iHour, TINT a_iMinute, TINT a_iSecond )
	{
		m_iHour    = a_iHour;
		m_bUseTime = TTRUE;
		m_iMinute  = a_iMinute;
		m_iSecond  = a_iSecond;
	}

	void SetUseTime( TBOOL a_bUseTime ) { m_bUseTime = a_bUseTime; }

private:
	// Moves the sun with the time of the day, updates the visibility and fills the meshes
	void Update();

	// Projects a_rcDirection to the screen, returns TFALSE if it's behind the camera or too far off the screen.
	// a_pOutVisible receives TFALSE if a_bCollide is set and the ray to the sun hits something.
	TBOOL GetScreenPosition( Toshi::TVector4& a_rOutPos, const Toshi::TVector4& a_rcDirection, TBOOL* a_pOutVisible, TBOOL a_bCollide, TFLOAT a_fWidth, TFLOAT a_fHeight );

	// Fills the quad of the sun or the moon
	void UpdateQuad( const Toshi::TVector4& a_rcScreenPos, TFLOAT a_fAlpha, TBOOL a_bMoon );

	// Fills the rays of the lens flare
	void UpdateFlare( const Toshi::TVector4& a_rcScreenPos, TFLOAT a_fAlpha );

private:
	Toshi::TVector4   m_vecSunDir;        // 0x00
	Toshi::TVector4   m_vecMoonDir;       // 0x10
	Toshi::TVector4   m_vecSunriseDir;    // 0x20, before 6:00 and after 20:00
	Toshi::TVector4   m_vecNoonDir;       // 0x30, from 11:00 to 15:00
	ASysMesh*         m_pSunMesh;         // 0x40
	ASysMaterial*     m_pSunMaterial;     // 0x44, "sun.tga"
	ASysMesh*         m_pFlareMesh;       // 0x48
	ASysMaterial*     m_pFlareMaterial;   // 0x4c, "corona.tga"
	ASysMesh*         m_pMoonMesh;        // 0x50
	ASysMaterial*     m_pMoonMaterial;    // 0x54, "moon.tga"
	TINT              m_iUnk58;           // 0x58
	TFLOAT            m_fSunIntensity;    // 0x5c
	TFLOAT            m_fSunSize;         // 0x60, relative to the screen width
	TFLOAT            m_fMoonSize;        // 0x64, relative to the screen height
	TFLOAT            m_fFadeOutTime;     // 0x68
	TFLOAT            m_fFadeSpeed;       // 0x6c
	TFLOAT            m_fMaxGlowTime;     // 0x70
	TFLOAT            m_fUnk74;           // 0x74
	TFLOAT            m_fFadeTime;        // 0x78
	TFLOAT            m_fGlowTime;        // 0x7c, how long the sun hasn't been hidden by anything
	TBOOL             m_bSunVisible;      // 0x80
	TBOOL             m_bMoonVisible;     // 0x81
	TBOOL             m_bSunUnoccluded;   // 0x82
	TUINT32           m_uiIgnoreGroups;   // 0x84, collision groups the ray to the sun ignores
	TFLOAT            m_fMoonIntensity;   // 0x88
	AMaterialLibrary* m_pMaterialLibrary; // 0x8c, Data\MiscTextures\SunflaretexWin.ttl
	TBOOL             m_bEnabled;         // 0x90
	TINT              m_iHour;            // 0x94
	TINT              m_iMinute;          // 0x98
	TINT              m_iSecond;          // 0x9c
	TBOOL             m_bUseTime;         // 0xa0
};
