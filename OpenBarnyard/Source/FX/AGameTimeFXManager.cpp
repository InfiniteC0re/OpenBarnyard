#include "pch.h"
#include "AGameTimeFXManager.h"
#include "AParticleEffectManager.h"
#include "Helpers/AGameLoader.h"
#include "Render/ARenderer.h"
#include "Render/ASkyDome.h"
#include "Render/ARainRenderer.h"
#include "Render/ASunFlareEffect.h"
#include "Terrain/ATerrainSection.h"

#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005441a0, CreateObject 00545f40, CreateObjectInPlace 00545fa0 (the constructor is inlined in both)
TDEFINE_CLASS( AGameTimeFXManager );

// $Barnyard: FUNCTION 00545d20
AGameTimeFXManager::AGameTimeFXManager()
{
	m_bBlending          = TFALSE;
	m_bForced            = TFALSE;
	m_bPaused            = TFALSE;
	m_bWasPaused         = TFALSE;
	m_fNextBlendDuration = -1.0f;
	m_bInJunkyard        = TFALSE;
	m_bJunkyardBlending  = TFALSE;
	m_bInWoods           = TFALSE;
	m_bInGulch           = TFALSE;
	m_bInMeadow          = TFALSE;
	m_bInBarn            = TFALSE;
	m_pAmbientEffect     = TNULL;
	m_pAmbientEffect2    = TNULL;
	m_eAmbientEffect     = AMBIENTEFFECT_NONE;
	m_eAmbientEffect2    = AMBIENTEFFECT_NONE;

	m_oDateChangeListener.Connect( *AGameTimeManager::GetSingleton()->GetDateChangeEmitter(), this, OnDateChangeCallback, 0 );

	// NOTE: the original rotates ( 0, 0, 1 ) by this quaternion and never uses the result
	TQuaternion quatSun( -0.046243802f, -0.477332f, -0.873416f, -0.084616303f );
	quatSun.AsVector4().Divide4( TMath::Sqrt( 0.9999997629005224f ) );

	TVector3 vecRotated;
	TQuaternion::RotateVector( vecRotated, quatSun, TVector3( 0.0f, 0.0f, 1.0f ) );

	TVector4 vecSunDir( vecRotated.x, 0.0f, vecRotated.y, 1.0f );
	vecSunDir.Normalise();

	m_oSettingsTRB.Load( "Data\\GameTimeFXSettings.trb" );
	LoadSettings();

	AGameTimeFX* pFirstFX = m_pppWeatherSettings[ AWeatherManager::WEATHERTYPE_NORMAL ][ AGameTimeManager::DAYPHASE_AFTERNOON ];
	m_oTo                 = *pFirstFX;
	m_oFrom               = *pFirstFX;
	m_oCurrent            = *pFirstFX;

	SetDayPhase( AGameTimeManager::DAYPHASE_MORNING, TTRUE, 0.0f );
	m_fBlendStart = 0.0f;
	m_fBlendTime  = m_fBlendDuration;
}

// $Barnyard: FUNCTION 005452e0
// $Barnyard: FUNCTION 00544ba0
AGameTimeFXManager::~AGameTimeFXManager()
{
	m_oDateChangeListener.Disconnect();
	StopAmbientEffects();
}

// $Barnyard: FUNCTION 00544d50
TBOOL AGameTimeFXManager::Reset()
{
	StopAmbientEffects();

	m_bInJunkyard       = TFALSE;
	m_bJunkyardBlending = TFALSE;
	m_bInWoods          = TFALSE;
	m_bInGulch          = TFALSE;
	m_bInMeadow         = TFALSE;
	m_bInBarn           = TFALSE;
	m_bWasPaused        = TFALSE;
	m_bPaused           = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00545630
TBOOL AGameTimeFXManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bPaused == TTRUE )
		return TFALSE;

	if ( m_bForced )
		return TTRUE;

	if ( m_bBlending )
	{
		m_fBlendTime += a_fDeltaTime;

		if ( m_fBlendTime < m_fBlendDuration )
		{
			if ( m_fBlendTime > m_fBlendStart )
			{
				TFLOAT fProgress = ( m_fBlendTime - m_fBlendStart ) * m_fBlendInvLength;
				m_oCurrent.Interpolate( m_oFrom, m_oTo, fProgress );

				if ( m_bUpdateSky )
				{
					const AGameTimeFX& rcSkyFX = m_bInJunkyard ? m_oJunkyardTo : m_oTo;

					ARenderer::GetSingleton()->GetSkyDome()->SetSkyColors(
					    m_fBlendDuration - m_fBlendStart,
					    &rcSkyFX.m_oSkyColorMatrix,
					    &rcSkyFX.m_oSkyLightMatrix,
					    rcSkyFX.m_fSkyBlend,
					    rcSkyFX.m_iSkyIndex,
					    m_fBlendTime - m_fBlendStart
					);

					m_bUpdateSky = TFALSE;
				}

				m_oJunkyardCurrent.Interpolate( m_oJunkyardFrom, m_oJunkyardTo, fProgress );
			}
		}
		else
		{
			m_bBlending = TFALSE;
			m_oCurrent  = m_oTo;
		}

		if ( !m_bJunkyardBlending )
		{
			if ( m_bInJunkyard )
				m_oJunkyardCurrent.Apply();
			else
				m_oCurrent.Apply();
		}
	}

	if ( !m_bJunkyardBlending )
		return TTRUE;

	m_fJunkyardBlendTime += a_fDeltaTime;

	if ( m_fJunkyardBlendTime < m_fJunkyardBlendDuration )
	{
		TFLOAT fProgress = m_fJunkyardBlendTime / m_fJunkyardBlendDuration;

		AGameTimeFX oFX;

		if ( !m_bInJunkyard )
			fProgress = 1.0f - fProgress;

		if ( m_bBlending )
			oFX.Interpolate( m_oCurrent, m_oJunkyardCurrent, fProgress );
		else
			oFX.Interpolate( m_oCurrent, m_oJunkyardTo, fProgress );

		oFX.Apply();
		return TTRUE;
	}

	m_bJunkyardBlending = TFALSE;

	if ( m_bBlending )
	{
		m_oJunkyardCurrent.Apply();
	}
	else if ( m_bInJunkyard )
	{
		m_oJunkyardCurrent = m_oJunkyardTo;
		m_oJunkyardTo.Apply();
	}
	else
	{
		m_oCurrent.Apply();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005458c0
void AGameTimeFXManager::SetDayPhase( AGameTimeManager::DAYPHASE a_eDayPhase, TBOOL a_bUnused, TFLOAT a_fElapsed )
{
	TFLOAT fDuration = m_fNextBlendDuration;

	m_bBlending          = TTRUE;
	m_bUpdateSky         = TTRUE;
	m_fNextBlendDuration = -1.0f;

	if ( fDuration <= 0.0f )
		fDuration = AGameTimeManager::DAYPHASE_DURATION[ a_eDayPhase ];

	m_fBlendTime     = ( a_fElapsed <= 0.0f ) ? 0.0f : a_fElapsed;
	m_fBlendDuration = fDuration;

	AWeatherManager* pWeather = AWeatherManager::GetSingleton();
	AGameTimeFX*     pFX      = m_pppWeatherSettings[ pWeather->GetCurrentWeather() ][ a_eDayPhase ];

	m_fBlendStart     = fDuration * pFX->m_fBlendStartFraction;
	m_fBlendInvLength = 1.0f / ( fDuration - m_fBlendStart );

	m_oCurrent = *pFX;

	const AGameTimeFX* pNextFX = m_pppWeatherSettings[ pWeather->GetNextWeather() ][ ( a_eDayPhase + 1 ) % AGameTimeManager::DAYPHASE_NUMOF ];
	m_oFrom                    = *pFX;
	m_oTo                      = *pNextFX;
	m_oJunkyardFrom            = m_oJunkyardCurrent;
	m_oJunkyardTo              = ( a_eDayPhase == AGameTimeManager::DAYPHASE_NIGHT ) ? *m_pJunkNight : *m_pJunkDay;

	if ( !m_bForced && !m_bInJunkyard )
	{
		ARenderer::GetSingleton()->GetSkyDome()->SetSkyColors(
		    0.0f,
		    &m_oFrom.m_oSkyColorMatrix,
		    &m_oFrom.m_oSkyLightMatrix,
		    m_oFrom.m_fSkyBlend,
		    m_oFrom.m_iSkyIndex,
		    0.0f
		);
	}

	UpdateAmbientEffects();
	Resume();
	OnUpdate( 0.0f );
	RestorePause();
}

// $Barnyard: FUNCTION 00544200
void AGameTimeFXManager::SetRemainingBlendTime( TFLOAT a_fDuration )
{
	TFLOAT fProgress = m_fBlendTime / m_fBlendDuration;
	TFLOAT fDuration = a_fDuration / ( 1.0f - fProgress );

	m_fBlendTime      = fDuration * fProgress;
	m_fBlendDuration  = fDuration;
	m_fBlendStart     = fDuration * m_oCurrent.m_fBlendStartFraction;
	m_fBlendInvLength = 1.0f / ( fDuration - m_fBlendStart );
}

// $Barnyard: FUNCTION 005445e0
AGameTimeFX* AGameTimeFXManager::FindFX( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_iNumSettings; i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_ppSettings[ i ]->m_szName, a_szName ) == 0 )
			return m_ppSettings[ i ];
	}

	return m_ppSettings[ 0 ];
}

// $Barnyard: FUNCTION 00545230
void AGameTimeFXManager::SetForcedFX( const AGameTimeFX* a_pFX )
{
	m_bForced = TTRUE;
	m_oForced = *a_pFX;

	ARenderer::GetSingleton()->GetSkyDome()->SetSkyColors(
	    0.0f,
	    &m_oForced.m_oSkyColorMatrix,
	    &m_oForced.m_oSkyLightMatrix,
	    m_oForced.m_fSkyBlend,
	    m_oForced.m_iSkyIndex,
	    0.0f
	);

	m_oForced.Apply();
}

// $Barnyard: FUNCTION 00545290
void AGameTimeFXManager::ClearForcedFX()
{
	m_bForced = TFALSE;
	m_oCurrent.Apply();

	ARenderer::GetSingleton()->GetSkyDome()->SetSkyColors(
	    0.0f,
	    &m_oCurrent.m_oSkyColorMatrix,
	    &m_oCurrent.m_oSkyLightMatrix,
	    m_oCurrent.m_fSkyBlend,
	    m_oCurrent.m_iSkyIndex,
	    0.0f
	);
}

// $Barnyard: FUNCTION 00544c30
void AGameTimeFXManager::Pause( TBOOL a_bRotateSky )
{
	m_bWasPaused = m_bPaused;
	m_bPaused    = TTRUE;

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_FROZEN );

	if ( a_bRotateSky )
	{
		pSkyDome->SetFlags( pSkyDome->GetFlags() | ASkyDome::FLAGS_ROTATE );
	}
	else
	{
		pSkyDome->SetFlags( pSkyDome->GetFlags() & ~ASkyDome::FLAGS_ROTATE );

		// NOTE: the original keeps the weather manager at ARenderer+0x80, here it's a singleton
		ARainRenderer::GetSingleton()->SetLockToCurrentWeatherType( TTRUE );
	}
}

// $Barnyard: FUNCTION 00544c90
void AGameTimeFXManager::Resume()
{
	m_bWasPaused = m_bPaused;
	m_bPaused    = TFALSE;

	ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
	pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_ROTATE );

	// NOTE: the original keeps the weather manager at ARenderer+0x80, here it's a singleton
	ARainRenderer::GetSingleton()->SetLockToCurrentWeatherType( TFALSE );
}

// $Barnyard: FUNCTION 00544ce0
void AGameTimeFXManager::RestorePause()
{
	if ( m_bPaused )
	{
		if ( !m_bWasPaused )
			Resume();
	}
	else if ( m_bWasPaused )
	{
		m_bWasPaused = TFALSE;
		m_bPaused    = TTRUE;

		ASkyDome* pSkyDome = ARenderer::GetSingleton()->GetSkyDome();
		pSkyDome->SetFlags( ( pSkyDome->GetFlags() & ~( ASkyDome::FLAGS_FROZEN | ASkyDome::FLAGS_ROTATE ) ) | ASkyDome::FLAGS_FROZEN );

		// NOTE: the original keeps the weather manager at ARenderer+0x80, here it's a singleton
		ARainRenderer::GetSingleton()->SetLockToCurrentWeatherType( TTRUE );
	}
}

// $Barnyard: FUNCTION 00545a70
void AGameTimeFXManager::OnVISGroupChanged( ATerrainSection* a_pCurrent, ATerrainSection* a_pPrevious )
{
	if ( !a_pCurrent )
		return;

	m_bInWoods  = TFALSE;
	m_bInGulch  = TFALSE;
	m_bInMeadow = TFALSE;
	m_bInBarn   = TFALSE;

	const TCHAR* szName = a_pCurrent->GetName();

	// NOTE: the original compares the name with "Junkyard" twice
	if ( TStringManager::String8CompareNoCase( szName, "Junkyard" ) != 0 && TStringManager::String8CompareNoCase( szName, "Junkyard" ) != 0 )
	{
		if ( TStringManager::String8CompareNoCase( szName, "Woods", 5 ) == 0 )
		{
			m_bInWoods = TTRUE;
		}
		else if ( TStringManager::String8CompareNoCase( szName, "JunkEntrance", 5 ) == 0 )
		{
			m_bInGulch = TTRUE;
		}
		else if ( TStringManager::String8CompareNoCase( szName, "Gulch", 5 ) == 0 )
		{
			m_bInGulch = TTRUE;
		}
		else if ( TStringManager::String8CompareNoCase( szName, "Barn", 4 ) == 0 || TStringManager::String8CompareNoCase( szName, "barnnorth", 9 ) == 0 )
		{
			m_bInBarn = TTRUE;
		}
		else if ( TStringManager::String8CompareNoCase( szName, "Hilltop" ) == 0 || TStringManager::String8CompareNoCase( szName, "Hillbase" ) == 0 || TStringManager::String8CompareNoCase( szName, "Meadowsouth" ) == 0 )
		{
			m_bInMeadow = TTRUE;
		}

		if ( m_bInJunkyard )
		{
			// Leaving the junkyard
			TBOOL bUpdateSky = m_bUpdateSky;

			m_fJunkyardBlendTime     = 0.0f;
			m_oJunkyardFrom          = m_oJunkyardCurrent;
			m_bInJunkyard            = TFALSE;
			m_bJunkyardBlending      = TTRUE;
			m_fJunkyardBlendDuration = JUNKYARD_BLEND_DURATION;
			m_oJunkyardTo            = m_oCurrent;

			const AGameTimeFX& rcSkyFX = bUpdateSky ? m_oFrom : m_oTo;

			ARenderer::GetSingleton()->GetSkyDome()->SetSkyColors(
			    JUNKYARD_BLEND_DURATION,
			    &rcSkyFX.m_oSkyColorMatrix,
			    &rcSkyFX.m_oSkyLightMatrix,
			    rcSkyFX.m_fSkyBlend,
			    rcSkyFX.m_iSkyIndex,
			    0.0f
			);

			UpdateAmbientEffects();
		}
	}
	else if ( !m_bInJunkyard )
	{
		// Entering the junkyard
		m_fJunkyardBlendTime     = 0.0f;
		m_oJunkyardCurrent       = m_oCurrent;
		m_oJunkyardFrom          = m_oCurrent;
		m_bInJunkyard            = TTRUE;
		m_bJunkyardBlending      = TTRUE;
		m_fJunkyardBlendDuration = JUNKYARD_BLEND_DURATION;
		m_oJunkyardTo            = ( AGameTimeManager::GetSingleton()->GetCurrentDayPhase() == AGameTimeManager::DAYPHASE_NIGHT ) ? *m_pJunkNight : *m_pJunkDay;

		ARenderer::GetSingleton()->GetSkyDome()->SetSkyColors(
		    JUNKYARD_BLEND_DURATION,
		    &m_oJunkyardTo.m_oSkyColorMatrix,
		    &m_oJunkyardTo.m_oSkyLightMatrix,
		    m_oJunkyardTo.m_fSkyBlend,
		    m_oJunkyardTo.m_iSkyIndex,
		    0.0f
		);

		UpdateJunkyardEffects();
	}
}

// $Barnyard: FUNCTION 00545300
void AGameTimeFXManager::UpdateAmbientEffects()
{
	TBOOL bMainGameLoaded = AGameLoader::IsMainGameLoaded();

	if ( !bMainGameLoaded && !AGameLoader::IsBarnExploreMiniGame() )
	{
		StopAmbientEffects();
		return;
	}

	AWeatherManager*  pWeather  = AWeatherManager::GetSingleton();
	AGameTimeManager* pGameTime = AGameTimeManager::GetSingleton();

	if ( m_bInJunkyard && !pWeather->IsBadWeather() )
	{
		UpdateJunkyardEffects();
		return;
	}

	// Stop the effects that don't match the current state
	TBOOL bStop = TFALSE;

	if ( m_eAmbientEffect == AMBIENTEFFECT_DANDELIONS && ( !bMainGameLoaded || !pGameTime->IsDaytime() || pWeather->IsBadWeather() ) )
		bStop = TTRUE;
	else if ( m_eAmbientEffect == AMBIENTEFFECT_FIREFLIES && ( !bMainGameLoaded || !pGameTime->IsItNight() || pWeather->IsBadWeather() ) )
		bStop = TTRUE;
	else if ( m_eAmbientEffect == AMBIENTEFFECT_DANDELIONSGUSTY && ( !bMainGameLoaded || !pWeather->IsBadWeather() ) )
		bStop = TTRUE;
	else if ( ( m_eAmbientEffect2 == AMBIENTEFFECT_LEAFSTORM && ( !bMainGameLoaded || !pWeather->IsHeavyRain() ) ) || m_eAmbientEffect == AMBIENTEFFECT_JUNKDUST || m_eAmbientEffect2 == AMBIENTEFFECT_JUNKTRASH )
		bStop = TTRUE;
	else if ( m_eAmbientEffect == AMBIENTEFFECT_BARNDUST || m_eAmbientEffect == AMBIENTEFFECT_NIGHTBARNSMOKE )
		bStop = !AGameLoader::IsBarnExploreMiniGame();

	if ( bStop )
		StopAmbientEffects();

	AParticleEffectManager* pParticleManager = AParticleEffectManager::GetSingleton();
	AMBIENTEFFECT           eEffect;
	TINT                    iEffect;

	if ( AGameLoader::IsBarnExploreMiniGame() )
	{
		if ( !pGameTime->IsItNight() )
		{
			if ( m_eAmbientEffect == AMBIENTEFFECT_BARNDUST )
				return;

			eEffect     = AMBIENTEFFECT_BARNDUST;
			m_iBarnDust = AParticleEffectManager::FindEffect( "barndust" );
			iEffect     = m_iBarnDust;
		}
		else
		{
			if ( m_eAmbientEffect == AMBIENTEFFECT_NIGHTBARNSMOKE )
				return;

			eEffect           = AMBIENTEFFECT_NIGHTBARNSMOKE;
			m_iNightBarnSmoke = AParticleEffectManager::FindEffect( "nightbarnsmoke" );
			iEffect           = m_iNightBarnSmoke;
		}
	}
	else if ( pWeather->IsBadWeather() )
	{
		if ( m_eAmbientEffect != AMBIENTEFFECT_DANDELIONSGUSTY )
		{
			m_iDandelionsGusty = AParticleEffectManager::FindEffect( "dandelionsgusty" );
			m_pAmbientEffect   = pParticleManager->PlayEffect( m_iDandelionsGusty, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( m_pAmbientEffect )
			{
				m_pAmbientEffect->AttachToCamera();
				m_eAmbientEffect = AMBIENTEFFECT_DANDELIONSGUSTY;
			}
		}

		if ( pWeather->IsHeavyRain() && m_eAmbientEffect2 != AMBIENTEFFECT_LEAFSTORM )
		{
			m_iLeafStorm      = AParticleEffectManager::FindEffect( "leafstorm" );
			m_pAmbientEffect2 = pParticleManager->PlayEffect( m_iLeafStorm, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

			if ( m_pAmbientEffect2 )
			{
				m_pAmbientEffect2->AttachToCamera();
				m_eAmbientEffect2 = AMBIENTEFFECT_LEAFSTORM;
			}
		}

		return;
	}
	else if ( pGameTime->IsDaytime() )
	{
		if ( m_eAmbientEffect == AMBIENTEFFECT_DANDELIONS )
			return;

		eEffect       = AMBIENTEFFECT_DANDELIONS;
		m_iDandelions = AParticleEffectManager::FindEffect( "dandelions" );
		iEffect       = m_iDandelions;
	}
	else
	{
		if ( !pGameTime->IsItNight() || m_eAmbientEffect == AMBIENTEFFECT_FIREFLIES )
			return;

		eEffect      = AMBIENTEFFECT_FIREFLIES;
		m_iFireflies = AParticleEffectManager::FindEffect( "fireflies" );
		iEffect      = m_iFireflies;
	}

	m_pAmbientEffect = pParticleManager->PlayEffect( iEffect, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

	if ( m_pAmbientEffect )
	{
		m_pAmbientEffect->AttachToCamera();
		m_eAmbientEffect = eEffect;
	}
}

// $Barnyard: FUNCTION 00544d90
void AGameTimeFXManager::UpdateJunkyardEffects()
{
	if ( AWeatherManager::GetSingleton()->IsBadWeather() )
	{
		UpdateAmbientEffects();
		return;
	}

	if ( m_eAmbientEffect != AMBIENTEFFECT_JUNKDUST || m_eAmbientEffect2 != AMBIENTEFFECT_JUNKTRASH )
		StopAmbientEffects();

	AParticleEffectManager* pParticleManager = AParticleEffectManager::GetSingleton();

	if ( m_eAmbientEffect != AMBIENTEFFECT_JUNKDUST )
	{
		m_iJunkDust      = AParticleEffectManager::FindEffect( "junkdust" );
		m_pAmbientEffect = pParticleManager->PlayEffect( m_iJunkDust, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

		if ( m_pAmbientEffect )
		{
			m_pAmbientEffect->AttachToCamera();
			m_eAmbientEffect = AMBIENTEFFECT_JUNKDUST;
		}
	}

	if ( m_eAmbientEffect2 != AMBIENTEFFECT_JUNKTRASH )
	{
		m_iJunkTrash      = AParticleEffectManager::FindEffect( "junktrash" );
		m_pAmbientEffect2 = pParticleManager->PlayEffect( m_iJunkTrash, TVector4::VEC_ZERO, TVector4::VEC_NEGZ );

		if ( m_pAmbientEffect2 )
		{
			m_pAmbientEffect2->AttachToCamera();
			m_eAmbientEffect2 = AMBIENTEFFECT_JUNKTRASH;
		}
	}
}

// $Barnyard: FUNCTION 005441b0
void AGameTimeFXManager::StopAmbientEffects()
{
	if ( m_eAmbientEffect != AMBIENTEFFECT_NONE )
	{
		m_pAmbientEffect->GetInstance()->Stop();
		m_pAmbientEffect = TNULL;
		m_eAmbientEffect = AMBIENTEFFECT_NONE;
	}

	if ( m_eAmbientEffect2 != AMBIENTEFFECT_NONE )
	{
		m_pAmbientEffect2->GetInstance()->Stop();
		m_pAmbientEffect2 = TNULL;
		m_eAmbientEffect2 = AMBIENTEFFECT_NONE;
	}
}

// $Barnyard: FUNCTION 00544710
void AGameTimeFXManager::LoadSettings()
{
	m_ppSettings         = m_oSettingsTRB.CastSymbol<AGameTimeFX*>( "settings" );
	m_iNumSettings       = 0;
	m_pppWeatherSettings = m_oSettingsTRB.CastSymbol<AGameTimeFX**>( "weathersettings" );
	m_pLightScale        = m_oSettingsTRB.CastSymbol<TVector4>( "lightscale" );

	m_vecShadowColorScale  = m_pLightScale[ 0 ];
	m_vecAmbientColorScale = m_pLightScale[ 1 ];

	// Scale the colours of the settings used by the weather
	for ( TINT i = 0; m_ppSettings[ i ] != TNULL; i++ )
	{
		for ( TINT iWeather = 0; iWeather < AWeatherManager::WEATHERTYPE_NUMOF; iWeather++ )
		{
			for ( TINT iDayPhase = 0; iDayPhase < AGameTimeManager::DAYPHASE_NUMOF; iDayPhase++ )
			{
				AGameTimeFX* pFX = m_pppWeatherSettings[ iWeather ][ iDayPhase ];

				if ( pFX == m_ppSettings[ i ] )
				{
					pFX->m_vecShadowColor.Multiply( m_vecShadowColorScale );
					pFX->m_vecAmbientColor.Multiply( m_vecAmbientColorScale );
					pFX->field7_0x70.Multiply( m_vecShadowColorScale );
					pFX->field8_0x80.Multiply( m_vecAmbientColorScale );

					iWeather = AWeatherManager::WEATHERTYPE_NUMOF;
					break;
				}
			}
		}
	}

	// Convert the colours from 0-255 to 0-1
	for ( ; m_ppSettings[ m_iNumSettings ] != TNULL; m_iNumSettings++ )
	{
		AGameTimeFX* pFX = m_ppSettings[ m_iNumSettings ];

		TMath::Clip( pFX->m_vecAmbientColor.x, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecAmbientColor.y, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecAmbientColor.z, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecShadowColor.x, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecShadowColor.y, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecShadowColor.z, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecFogColor.x, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecFogColor.y, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecFogColor.z, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecSkyTint.x, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecSkyTint.y, 0.0f, 255.0f );
		TMath::Clip( pFX->m_vecSkyTint.z, 0.0f, 255.0f );
		TMath::Clip( pFX->field7_0x70.x, 0.0f, 255.0f );
		TMath::Clip( pFX->field7_0x70.y, 0.0f, 255.0f );
		TMath::Clip( pFX->field7_0x70.z, 0.0f, 255.0f );
		TMath::Clip( pFX->field8_0x80.x, 0.0f, 255.0f );
		TMath::Clip( pFX->field8_0x80.y, 0.0f, 255.0f );
		TMath::Clip( pFX->field8_0x80.z, 0.0f, 255.0f );
		TMath::Clip( pFX->m_fLightIntensity, 0.0f, 1.0f );

		pFX->m_vecShadowColor.Multiply4( 1.0f / 255.0f );
		pFX->m_vecAmbientColor.Multiply4( 1.0f / 255.0f );
		pFX->m_vecFogColor.Multiply4( 1.0f / 255.0f );
		pFX->m_vecSkyTint.Multiply4( 1.0f / 255.0f );
		pFX->field7_0x70.Multiply4( 1.0f / 255.0f );
		pFX->field8_0x80.Multiply4( 1.0f / 255.0f );

		pFX->m_vecLightDir.Normalise();
		pFX->m_vecSunFlareDir.Normalise();

		pFX->m_oSkyColorMatrix.AsBasisVector4( 3 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyColorMatrix.AsBasisVector4( 1 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyColorMatrix.AsBasisVector4( 2 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyColorMatrix.AsBasisVector4( 0 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyLightMatrix.AsBasisVector4( 3 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyLightMatrix.AsBasisVector4( 1 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyLightMatrix.AsBasisVector4( 2 ).Multiply4( 1.0f / 255.0f );
		pFX->m_oSkyLightMatrix.AsBasisVector4( 0 ).Multiply4( 1.0f / 255.0f );
	}

	m_oSettingsTRB.DeleteSymbolTable();

	m_pJunkDay   = FindFX( "JunkDay" );
	m_pJunkNight = FindFX( "JunkNight" );

	ASunFlareEffect::GetSingleton()->SetSunDirections(
	    m_pppWeatherSettings[ AWeatherManager::WEATHERTYPE_NORMAL ][ AGameTimeManager::DAYPHASE_DAWN ]->m_vecLightDir,
	    m_pppWeatherSettings[ AWeatherManager::WEATHERTYPE_NORMAL ][ AGameTimeManager::DAYPHASE_AFTERNOON ]->m_vecLightDir
	);

	// Every setting uses the light direction of the normal afternoon
	TVector4 vecLightDir = m_pppWeatherSettings[ AWeatherManager::WEATHERTYPE_NORMAL ][ AGameTimeManager::DAYPHASE_AFTERNOON ]->m_vecLightDir;

	for ( TINT iWeather = 0; iWeather < AWeatherManager::WEATHERTYPE_NUMOF; iWeather++ )
	{
		for ( TINT iDayPhase = 0; iDayPhase < AGameTimeManager::DAYPHASE_NUMOF; iDayPhase++ )
			m_pppWeatherSettings[ iWeather ][ iDayPhase ]->m_vecLightDir = vecLightDir;
	}
}

// $Barnyard: FUNCTION 00545d00
TBOOL AGameTimeFXManager::OnDateChangeCallback( AGameTimeFXManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent )
{
	if ( a_pEvent->m_eUpdateType == AGameTimeManager::DATECHANGETYPE_DAYPHASE )
		a_pCaller->SetDayPhase( a_pEvent->m_eCurrDayPhase, TFALSE, 0.0f );

	return TTRUE;
}
