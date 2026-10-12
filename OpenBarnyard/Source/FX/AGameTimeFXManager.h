#pragma once
#include "AGameTimeFX.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AWeatherManager.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <File/TTRB.h>

class AParticleEffectHandle;
class ATerrainSection;

// Blends the lighting, fog, sky and rain settings (AGameTimeFX) over the day
// phases and the weather, applies a different set inside the junkyard and plays
// the ambient particle effects (dandelions, fireflies, leaves, dust...) that
// follow the camera.
//
// Singleton at 0x00783d08, vtable 0x0072404c.
class AGameTimeFXManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AGameTimeFXManager>
{
public:
	TDECLARE_CLASS( AGameTimeFXManager, Toshi::TTask );

	using AMBIENTEFFECT = TINT;
	enum AMBIENTEFFECT_ : AMBIENTEFFECT
	{
		AMBIENTEFFECT_NONE,
		AMBIENTEFFECT_DANDELIONS,
		AMBIENTEFFECT_FIREFLIES,
		AMBIENTEFFECT_DANDELIONSGUSTY,
		AMBIENTEFFECT_LEAFSTORM,
		AMBIENTEFFECT_JUNKDUST,
		AMBIENTEFFECT_JUNKTRASH,
		AMBIENTEFFECT_BARNDUST,
		AMBIENTEFFECT_NIGHTBARNSMOKE,
	};

	// Number of the junkyard transition seconds
	inline static constexpr TFLOAT JUNKYARD_BLEND_DURATION = 10.0f;

public:
	AGameTimeFXManager();
	~AGameTimeFXManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Starts blending to the settings of a_eDayPhase (and the weather). a_fElapsed is the time already spent in the phase.
	void SetDayPhase( AGameTimeManager::DAYPHASE a_eDayPhase, TBOOL a_bUnused, TFLOAT a_fElapsed );

	// Changes the remaining time of the current day phase blend to a_fDuration keeping its progress
	void SetRemainingBlendTime( TFLOAT a_fDuration );

	// Returns the settings named a_szName or the first settings if there are no such settings
	AGameTimeFX* FindFX( const TCHAR* a_szName );

	// Stops blending and applies a_pFX until ClearForcedFX is called
	void SetForcedFX( const AGameTimeFX* a_pFX );
	void ClearForcedFX();

	// Freezes the blending, a_bRotateSky keeps the clouds of the sky moving
	void Pause( TBOOL a_bRotateSky );
	void Resume();

	// Restores the paused state that was there before the last Pause/Resume
	void RestorePause();

	// Updates the region flags and blends in or out of the junkyard settings
	void OnVISGroupChanged( ATerrainSection* a_pCurrent, ATerrainSection* a_pPrevious );

	// Starts the ambient particle effects for the current weather, time and region
	void UpdateAmbientEffects();
	void StopAmbientEffects();

	void  SetInBarn( TBOOL a_bInBarn ) { m_bInBarn = a_bInBarn; }
	TBOOL IsInBarn() const { return m_bInBarn; }
	TBOOL IsInJunkyard() const { return m_bInJunkyard; }
	TBOOL IsInWoods() const { return m_bInWoods; }
	TBOOL IsInGulch() const { return m_bInGulch; }
	TBOOL IsInMeadow() const { return m_bInMeadow; }

	void SetNextBlendDuration( TFLOAT a_fDuration ) { m_fNextBlendDuration = a_fDuration; }

	AGameTimeFX** GetSettings() const { return m_ppSettings; }
	AGameTimeFX*  GetWeatherSettings( AWeatherManager::WEATHERTYPE a_eWeather, AGameTimeManager::DAYPHASE a_eDayPhase ) const { return m_pppWeatherSettings[ a_eWeather ][ a_eDayPhase ]; }

private:
	void LoadSettings();
	void UpdateJunkyardEffects();

	static TBOOL __stdcall OnDateChangeCallback( AGameTimeFXManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent );

private:
	Toshi::TListener<AGameTimeManager, AGameTimeManager::DateChangeEvent, AGameTimeFXManager> m_oDateChangeListener;    // 0x20
	TBOOL                                                                                     m_bBlending;              // 0x34
	TFLOAT                                                                                    m_fBlendTime;             // 0x38
	TFLOAT                                                                                    m_fBlendDuration;         // 0x3c
	TFLOAT                                                                                    m_fBlendStart;            // 0x40
	TFLOAT                                                                                    m_fBlendInvLength;        // 0x44
	Toshi::TVector4*                                                                          m_pLightScale;            // 0x48
	Toshi::TVector4                                                                           m_vecShadowColorScale;    // 0x4c
	Toshi::TVector4                                                                           m_vecAmbientColorScale;   // 0x5c
	AGameTimeFX                                                                               m_oFrom;                  // 0x6c
	AGameTimeFX                                                                               m_oTo;                    // 0x1e4
	AGameTimeFX                                                                               m_oCurrent;               // 0x35c
	TBOOL                                                                                     m_bForced;                // 0x4d4
	AGameTimeFX                                                                               m_oForced;                // 0x4d8
	TBOOL                                                                                     m_bUpdateSky;             // 0x650
	Toshi::TTRB                                                                               m_oSettingsTRB;           // 0x654
	AGameTimeFX**                                                                             m_ppSettings;             // 0x794, null terminated
	AGameTimeFX***                                                                            m_pppWeatherSettings;     // 0x798, [ weather ][ day phase ]
	TINT                                                                                      m_iNumSettings;           // 0x79c
	TBOOL                                                                                     m_bPaused;                // 0x7a0
	TBOOL                                                                                     m_bWasPaused;             // 0x7a1
	TFLOAT                                                                                    m_fNextBlendDuration;     // 0x7a4
	TBOOL                                                                                     m_bInJunkyard;            // 0x7a8
	TBOOL                                                                                     m_bJunkyardBlending;      // 0x7a9
	TFLOAT                                                                                    m_fJunkyardBlendTime;     // 0x7ac
	TFLOAT                                                                                    m_fJunkyardBlendDuration; // 0x7b0
	AGameTimeFX                                                                               m_oJunkyardFrom;          // 0x7b4
	AGameTimeFX                                                                               m_oJunkyardCurrent;       // 0x92c
	AGameTimeFX                                                                               m_oJunkyardTo;            // 0xaa4
	TBOOL                                                                                     m_bInWoods;               // 0xc1c
	TBOOL                                                                                     m_bInGulch;               // 0xc1d
	TBOOL                                                                                     m_bInMeadow;              // 0xc1e
	TBOOL                                                                                     m_bInBarn;                // 0xc1f
	AGameTimeFX*                                                                              m_pJunkDay;               // 0xc20
	AGameTimeFX*                                                                              m_pJunkNight;             // 0xc24
	AParticleEffectHandle*                                                                    m_pAmbientEffect;         // 0xc28
	AParticleEffectHandle*                                                                    m_pAmbientEffect2;        // 0xc2c
	AMBIENTEFFECT                                                                             m_eAmbientEffect;         // 0xc30
	AMBIENTEFFECT                                                                             m_eAmbientEffect2;        // 0xc34
	TINT                                                                                      m_iNightBarnSmoke;        // 0xc38
	TINT                                                                                      m_iBarnDust;              // 0xc3c
	TINT                                                                                      m_iDandelionsGusty;       // 0xc40
	TINT                                                                                      m_iLeafStorm;             // 0xc44
	TINT                                                                                      m_iDandelions;            // 0xc48
	TINT                                                                                      m_iFireflies;             // 0xc4c
	TINT                                                                                      m_iJunkDust;              // 0xc50
	TINT                                                                                      m_iJunkTrash;             // 0xc54
};
