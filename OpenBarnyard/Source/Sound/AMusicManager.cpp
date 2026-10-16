#include "pch.h"
#include "AMusicManager.h"
#include "AMultiplexStream.h"
#include "ASoundManager.h"
#include "FX/AGameTimeFXManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AMiniGameInstructionState.h"
#include "GameInterface/AStartMenuState.h"
#include "Helpers/ABikeManager.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AWeatherManager.h"
#include "Quests/AQuestManager.h"
#include "Tasks/ARootTask.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

#include <Toshi/TSystem.h>
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( mission );
TPSTRING8_DECLARE( mission02 );
TPSTRING8_DECLARE( mission03 );
TPSTRING8_DECLARE( mission04 );
TPSTRING8_DECLARE( BDeath );
TPSTRING8_DECLARE( BY_NIGHT );
TPSTRING8_DECLARE( BY_DAY );
TPSTRING8_DECLARE( Bike );
#ifdef BARNYARD_LATEST
TPSTRING8_DECLARE( Bike02 );
TPSTRING8_DECLARE( BY_day02 );
TPSTRING8_DECLARE( BY_day03 );
TPSTRING8_DECLARE( BY_day04 );
TPSTRING8_DECLARE( BY_day05 );
TPSTRING8_DECLARE( BY_day06 );
#endif
TPSTRING8_DECLARE( AAudioOptionsState );

// NOTE: GetClass 005d45c0, CreateObject 005d5630, CreateObjectInPlace 005d5690
TDEFINE_CLASS( AMusicManager );

// Durations of the fades of the main game streams
static constexpr TFLOAT s_fBikeFadeOutTime    = 1.0f; // 0x007753cc
static constexpr TFLOAT s_fDefaultFadeOutTime = 2.0f; // 0x007753d0
static constexpr TFLOAT s_fDefaultFadeTime    = 1.0f; // 0x007753d4

// NOTE: zero initialised and never written in retail, the bike and the mission streams start at full volume
static TFLOAT s_fBikeFadeInTime = 0.0f; // 0x0078c3f8

// The sound of the rain over the music
static constexpr ASoundId RAIN_SOUND = 188;

// $Barnyard: FUNCTION 005d4760
AMusicManager::AMusicManager()
    : m_bNearBenGrave( TFALSE )
    , m_pLastState( TNULL )
    , m_iLastDayPhase( 5 )
    , m_iLastNumChildStates( 0 )
    , m_bEnabled( TFALSE )
    , m_fVolume( 1.0f )
    , m_iCue( -1 )
    , m_iSound( -1 )
    , m_iBackgroundMusicCue( -1 )
    , m_iForcedSound( -1 )
    , m_fFadeTime( 0.0f )
    , m_fFadeDuration( 0.0f )
    , m_bFadingOut( TFALSE )
    , m_bFadingIn( TFALSE )
    , m_bForceUpdate( TFALSE )
    , m_iSavedStreamTime( -1 )
    , m_iRainCue( -1 )
{
}

// $Barnyard: FUNCTION 005d47c0
// $Barnyard: FUNCTION 005d56d0
AMusicManager::~AMusicManager()
{
}

// $Barnyard: FUNCTION 005d53f0
TBOOL AMusicManager::Reset()
{
	m_iCue = -1;

	if ( ASoundManager::IsSingletonCreated() )
		ASoundManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	m_strCurrentStream = TPString8();
	m_strMissionStream = TPString8();
	m_bFadingOut       = TFALSE;
	m_bFadingIn        = TFALSE;
	m_fFadeTime        = 0.0f;
	m_iSavedStreamTime = -1;

	if ( AGameLoader::IsMainGameLoaded() == TTRUE && AMultiplexStream::GetSingleton()->GetState() != AMultiplexStream::STATE_CLOSED )
		AMultiplexStream::GetSingleton()->Close( TTRUE );

	m_pLastState   = TNULL;
	m_iForcedSound = -1;

	return TTask::Reset();
}

#ifdef BARNYARD_LATEST

// The Wii day and bike music: a random track each time the current one has ended. A track the
// PC data doesn't have (no converted Wii streams, see AMultiplexStream::LoadWiiStreams) falls
// back to the retail one, so without the Wii data this plays like PC.
static const TPString8& PickTrack( TPString8& a_rCurrent, const TPString8* const* a_ppTracks, TINT a_iNumTracks, const TPString8& a_rcFallback )
{
	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( a_rCurrent.IsEmpty() || ( pStream->GetState() == AMultiplexStream::STATE_PLAYING && !pStream->IsChannelPlaying() && pStream->IsCurrentStream( a_rCurrent ) ) )
	{
		const TPString8& rcTrack = *a_ppTracks[ ARandom::GetSingleton()->m_oRandom.GetInt( a_iNumTracks ) ];
		a_rCurrent               = pStream->HasStream( rcTrack.GetString() ) ? rcTrack : a_rcFallback;
	}

	return a_rCurrent;
}

#endif

// $Barnyard: FUNCTION 005d4860
// $BarnyardWiiJP: FUNCTION 8036a55c
TBOOL AMusicManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_bNearBenGrave = TFALSE;

#if defined( BARNYARD_LATEST ) && defined( BARNYARD_COMMUNITY_PATCH )
	// Off the bike: the next ride picks its track again
	if ( !ABikeManager::IsSingletonCreated() || !ABikeManager::GetSingleton()->IsPlayerOnBike() )
		m_strBikeStream = TPString8();
#endif

	if ( !m_bEnabled )
		return TTask::OnUpdate( a_fDeltaTime );

	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	if ( AGameLoader::IsMainGameLoaded() != TTRUE )
	{
		// The music is the sound of the current game state
		AGameState* pState    = AGameStateController::GetSingleton()->GetCurrentState();
		TINT        iDayPhase = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();

		if ( ARootTask::GetSingleton()->IsPaused() != TTRUE || g_oSystemManager.IsPaused() )
		{
			// The music continues where it was before the start menu
			if ( m_iCue != -1 && m_iSavedStreamTime != -1 )
			{
				if ( pSoundManager->IsCueStreamReady( m_iCue ) == TTRUE && pSoundManager->IsStreamCueActive( m_iCue ) == TTRUE )
				{
					pSoundManager->SetStreamCueTime( m_iCue, m_iSavedStreamTime );
					m_iSavedStreamTime = -1;
				}
			}

			TINT     iNumChildStates = pState->GetNumChildStates();
			ASoundId iStateSound     = pState->GetSound();
			ASoundId iChildSound     = pState->GetChildStatesSound();

			if ( m_bForceUpdate ||
			     ( ( pState != m_pLastState || iDayPhase != m_iLastDayPhase || iNumChildStates != m_iLastNumChildStates ) && ( iStateSound != -1 || iChildSound != -1 ) ) )
			{
				ASoundId iPreviousSound = m_iSound;
				m_bForceUpdate          = TFALSE;

				if ( m_iForcedSound == -1 )
					m_iSound = ( iChildSound == -1 ) ? iStateSound : iChildSound;
				else
					m_iSound = m_iForcedSound;

				if ( iPreviousSound != m_iSound )
				{
					pSoundManager->StopCue( m_iCue );
					m_iCue = pSoundManager->PlayCueEx( m_iSound, 0.0f, TFALSE, 0.0f, -1 );
					pSoundManager->FadeCue( m_iCue, 1.0f, 1.0f, 20.0f );
				}
			}

			m_iLastNumChildStates = iNumChildStates;
			m_pLastState          = pState;
			m_iLastDayPhase       = iDayPhase;

			if ( m_iCue != -1 && m_fVolume != pSoundManager->GetCueVolume( m_iCue ) )
				pSoundManager->SetCueVolume( m_iCue, m_fVolume, 0.0f );
		}

		return TTask::OnUpdate( a_fDeltaTime );
	}

	// The main game: a multiplex stream
	AMultiplexStream* pStream          = AMultiplexStream::GetSingleton();
	AWeatherManager*  pWeather         = AWeatherManager::GetSingleton();
	TBOOL             bPauseWhenSilent = TFALSE;
	TPString8         strStream;

	if ( m_iForcedSound == -1 )
	{
		if ( m_iCue != -1 )
			pSoundManager->StopCue( m_iCue );

		if ( ARootTask::GetSingleton()->IsPaused() == TTRUE )
		{
			if ( !AMultiplexStream::IsPaused() && !g_oSystemManager.IsPaused() )
				pStream->Pause( TTRUE );

			return TTask::OnUpdate( a_fDeltaTime );
		}

		AQuestManager* pQuestManager = AQuestManager::GetSingleton();

		if ( !pQuestManager->IsMissionMusicPlaying() )
			m_strMissionStream = TPString8();

		if ( pQuestManager->IsMissionMusicPlaying() == TTRUE )
		{
			// A quest sequence: one of the mission streams
			if ( m_strMissionStream.IsEmpty() )
			{
				switch ( ARandom::GetSingleton()->m_oRandom.GetInt( 4 ) )
				{
					case 1:
						m_strMissionStream = TPS8( mission02 );
						break;
					case 2:
						m_strMissionStream = TPS8( mission03 );
						break;
					case 3:
						m_strMissionStream = TPS8( mission04 );
						break;
					default:
						m_strMissionStream = TPS8( mission );
						break;
				}
			}

			strStream = m_strMissionStream;
		}
		else
		{
			AGameTimeFXManager* pGameTimeFXManager = AGameTimeFXManager::GetSingleton();

			if ( pGameTimeFXManager->IsInJunkyard() || pGameTimeFXManager->IsInGulch() || pGameTimeFXManager->IsInWoods() )
			{
				// No music there
				bPauseWhenSilent = TTRUE;
			}
			else
			{
				if ( AGameTimeManager::GetSingleton()->IsItNight() )
				{
					TINT  iAct        = pQuestManager->GetAct();
					TBOOL bLateInGame = iAct == 3 || ( iAct == 2 && pQuestManager->GetTier() > 18 );

					if ( pGameTimeFXManager->IsInMeadow() && bLateInGame )
					{
						// Ben's grave
						strStream       = TPS8( BDeath );
						m_bNearBenGrave = TTRUE;
					}
					else if ( !pGameTimeFXManager->IsInBarn() )
					{
						strStream = TPS8( BY_NIGHT );
					}
				}
				else if ( ABikeManager::GetSingleton()->IsPlayerOnBike() == TTRUE )
				{
#ifdef BARNYARD_LATEST
#  ifdef BARNYARD_COMMUNITY_PATCH
					// NOTE: community patch: a new random bike track for each ride (m_strBikeStream is cleared at the top of OnUpdate while off the bike).
					// The Wii only picks again once the track has ended, and a ride is usually shorter than the song, so the
					// first pick of the session (PC's Bike or the Wii's Bike02) played on every ride
#  endif
					static const TPString8* s_apBikeTracks[] = { &TPS8( Bike ), &TPS8( Bike02 ) };
					strStream                                = PickTrack( m_strBikeStream, s_apBikeTracks, TARRAYSIZE( s_apBikeTracks ), TPS8( Bike ) );
#else
					strStream = TPS8( Bike );
#endif
				}
				else
				{
#ifdef BARNYARD_LATEST
					static const TPString8* s_apDayTracks[] = { &TPS8( BY_DAY ), &TPS8( BY_day02 ), &TPS8( BY_day03 ), &TPS8( BY_day04 ), &TPS8( BY_day05 ), &TPS8( BY_day06 ) };
					strStream                               = PickTrack( m_strDayStream, s_apDayTracks, TARRAYSIZE( s_apDayTracks ), TPS8( BY_DAY ) );
#else
					strStream = TPS8( BY_DAY );
#endif
				}
			}
		}

		// Resume the stream (also for the mission music), unless the area is silent
		if ( !bPauseWhenSilent && AMultiplexStream::IsPaused() == TTRUE )
			pStream->Pause( TFALSE );
	}
	else if ( pStream->GetState() == AMultiplexStream::STATE_CLOSED )
	{
		// The forced sound plays instead of the stream
		m_bFadingIn        = TFALSE;
		m_bFadingOut       = TFALSE;
		m_strCurrentStream = TPString8();
		m_strMissionStream = TPString8();
		m_fFadeTime        = 0.0f;
		m_iSavedStreamTime = -1;

		ASoundId iPreviousSound = m_iSound;

		if ( m_iForcedSound != -1 )
			m_iSound = m_iForcedSound;

		if ( iPreviousSound != m_iSound )
		{
			pSoundManager->StopCue( m_iCue );
			m_iCue = pSoundManager->PlayCue( m_iSound );
		}
	}

	if ( strStream != m_strCurrentStream )
	{
		// Changing the stream: the current one fades out first
		if ( m_bNearBenGrave )
		{
			// NOTE: retail uses AInteriorNetwork::FindGoalOrFirst (00553780), FindGoal without the assert
			AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();
			pNetwork->FindGoal( TPString8( "park2_goals_ben_grave" ) )->GetPosition( m_vecBenGravePos );
		}

		m_bFadingIn = TFALSE;

		switch ( pStream->GetState() )
		{
			case AMultiplexStream::STATE_PLAYING:
				if ( !m_bFadingOut )
				{
					// NOTE: the Wii (8036a55c) also only checks Bike here and for the fade in, not Bike02
					m_bFadingOut    = TTRUE;
					m_fFadeDuration = ( strStream == TPS8( Bike ) ) ? s_fBikeFadeOutTime : s_fDefaultFadeOutTime;

					if ( m_fVolume <= 0.0f )
						m_fFadeTime = 0.0f;
					else
						m_fFadeTime = ( 1.0f - pStream->GetVolume( 0 ) / m_fVolume ) * m_fFadeDuration;

					if ( !ARootTask::GetSingleton()->IsPaused() && ( pWeather->IsHeavyRain() || pWeather->IsBadWeather() ) )
					{
						// The rain gets louder
						if ( m_iRainCue == -1 )
							m_iRainCue = pSoundManager->PlayCueEx( RAIN_SOUND, 0.0f, TFALSE, 0.0f, -1 );
						else
							pSoundManager->SetCueVolume( m_iRainCue, 1.0f, 0.0f );

						pSoundManager->FadeCue( m_iRainCue, m_fFadeDuration - m_fFadeTime, 1.0f, 3.0f );
					}
				}

				if ( m_bFadingOut == TTRUE )
				{
					m_fFadeTime += a_fDeltaTime;
					TFLOAT fVolume;

					if ( m_fFadeDuration <= 0.0f )
					{
						fVolume = 0.0f;
					}
					else
					{
						fVolume = ( 1.0f - m_fFadeTime / m_fFadeDuration ) * m_fVolume;
						TMath::Clip( fVolume, 0.0f, m_fVolume );
					}

					pStream->SetVolume( fVolume, 0, TTRUE );
					pStream->SetVolume( fVolume, 1, TTRUE );

					// The rain layers
					if ( !ARootTask::GetSingleton()->IsPaused() && ( pWeather->IsHeavyRain() || pWeather->IsBadWeather() ) )
					{
						pStream->SetVolume( fVolume, 2, TTRUE );
						pStream->SetVolume( fVolume, 3, TTRUE );
					}
					else
					{
						pStream->SetVolume( 0.0f, 2, TTRUE );
						pStream->SetVolume( 0.0f, 3, TTRUE );
					}

					if ( fVolume == 0.0f )
					{
						if ( bPauseWhenSilent )
						{
							if ( !AMultiplexStream::IsPaused() )
								pStream->Pause( TTRUE );
						}
						else
						{
							m_bFadingOut = TFALSE;
							m_fFadeTime  = 0.0f;
							pStream->Stop();
						}
					}
				}

				break;
			case AMultiplexStream::STATE_READY:
				pStream->Close( TFALSE );
				break;
			case AMultiplexStream::STATE_CLOSED:
				m_strCurrentStream = strStream;
				break;
		}
	}
	else if ( !m_strCurrentStream.IsEmpty() )
	{
		m_bFadingOut = TFALSE;

		AMultiplexStream::STATE eState = pStream->GetState();

		if ( eState == AMultiplexStream::STATE_CLOSED )
		{
			pStream->Open( m_strCurrentStream.GetString() );
		}
		else if ( eState == AMultiplexStream::STATE_READY )
		{
			pStream->Play( 0.0f );

			if ( m_bFadingIn )
			{
				if ( ARootTask::GetSingleton()->IsPaused() == TTRUE && AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( AAudioOptionsState ) ) )
					pStream->Update( a_fDeltaTime );

				return TTask::OnUpdate( a_fDeltaTime );
			}

			if ( !m_bNearBenGrave )
			{
				m_bFadingIn = TTRUE;

				if ( m_strCurrentStream == TPS8( Bike ) || m_strCurrentStream == TPS8( mission ) )
					m_fFadeDuration = s_fBikeFadeInTime;
				else
					m_fFadeDuration = s_fDefaultFadeTime;

				if ( m_fVolume <= 0.0f )
					m_fFadeTime = m_fFadeDuration;
				else
					m_fFadeTime = pStream->GetVolume( 0 ) / m_fVolume * m_fFadeDuration;
			}
		}
		else if ( eState == AMultiplexStream::STATE_PLAYING && !m_bNearBenGrave )
		{
			TFLOAT fVolume;

			if ( m_bFadingIn == TTRUE )
			{
				// Fading in, the rain gets quieter
				if ( m_iRainCue != -1 )
				{
					pSoundManager->FadeOutCue( m_iRainCue, m_fFadeDuration - m_fFadeTime, TTRUE, 3.0f );
					m_iRainCue = -1;
				}

				m_fFadeTime += a_fDeltaTime;

				if ( m_fFadeDuration <= 0.0f )
				{
					fVolume = 1.0f;
				}
				else
				{
					fVolume = m_fFadeTime / m_fFadeDuration * m_fVolume;
					TMath::Clip( fVolume, 0.0f, m_fVolume );
				}

				pStream->SetVolume( fVolume, 0, TTRUE );
				pStream->SetVolume( fVolume, 1, TTRUE );

				if ( !ARootTask::GetSingleton()->IsPaused() && ( pWeather->IsHeavyRain() || pWeather->IsBadWeather() ) )
				{
					pStream->SetVolume( fVolume, 2, TTRUE );
					pStream->SetVolume( fVolume, 3, TTRUE );
				}
				else
				{
					pStream->SetVolume( 0.0f, 2, TTRUE );
					pStream->SetVolume( 0.0f, 3, TTRUE );
				}

				if ( fVolume >= m_fVolume )
				{
					m_bFadingIn = TFALSE;
					m_fFadeTime = 0.0f;
				}
			}
			else
			{
				// Follows the volume of the music
				TFLOAT fCurrentVolume = pStream->GetVolume( 0 );
				TBOOL  bChange        = TTRUE;

				if ( fCurrentVolume >= m_fVolume )
				{
					if ( fCurrentVolume <= m_fVolume )
					{
						bChange = TFALSE;
					}
					else
					{
						fVolume = fCurrentVolume - a_fDeltaTime;

						if ( fVolume < m_fVolume )
							fVolume = m_fVolume;
					}
				}
				else
				{
					fVolume = fCurrentVolume + a_fDeltaTime;

					if ( fVolume > m_fVolume )
						fVolume = m_fVolume;
				}

				if ( bChange )
				{
					pStream->SetVolume( fVolume, 0, TTRUE );
					pStream->SetVolume( fVolume, 1, TTRUE );

					if ( !ARootTask::GetSingleton()->IsPaused() && ( pWeather->IsHeavyRain() || pWeather->IsBadWeather() ) )
					{
						pStream->SetVolume( fVolume, 2, TTRUE );
						pStream->SetVolume( fVolume, 3, TTRUE );
					}
					else
					{
						pStream->SetVolume( 0.0f, 2, TTRUE );
						pStream->SetVolume( 0.0f, 3, TTRUE );
					}
				}
			}
		}
	}

	// The rain layers
	if ( !m_bFadingIn && !m_bFadingOut )
	{
		if ( !ARootTask::GetSingleton()->IsPaused() && ( pWeather->IsHeavyRain() || pWeather->IsBadWeather() ) )
		{
			if ( pStream->GetState() == AMultiplexStream::STATE_PLAYING )
			{
				pStream->SetVolume( 1.0f, 2, TTRUE );
				pStream->SetVolume( 1.0f, 3, TTRUE );
			}
		}
		else if ( pStream->GetState() == AMultiplexStream::STATE_PLAYING )
		{
			pStream->SetVolume( 0.0f, 2, TTRUE );
			pStream->SetVolume( 0.0f, 3, TTRUE );
		}
	}

	// The game is paused in the audio options, keep the stream updated so the volume can be heard
	if ( ARootTask::GetSingleton()->IsPaused() == TTRUE && AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( AAudioOptionsState ) ) )
		pStream->Update( a_fDeltaTime );

	return TTask::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 005d46b0
void AMusicManager::PlayBackgroundMusic( ASoundId a_eSoundId )
{
	if ( m_iBackgroundMusicCue == -1 )
		m_iBackgroundMusicCue = ASoundManager::GetSingleton()->PlayCue( a_eSoundId );
}

// $Barnyard: FUNCTION 005d46d0
void AMusicManager::StopBackgroundMusic()
{
	ASoundManager::GetSingleton()->StopCue( m_iBackgroundMusicCue );
	m_iBackgroundMusicCue = -1;
}

// $Barnyard: FUNCTION 005d4600
void AMusicManager::Restart()
{
	if ( AGameLoader::IsMainGameLoaded() == TTRUE )
	{
		OnUpdate( 0.0f );
		return;
	}

	if ( m_iCue != -1 )
		ASoundManager::GetSingleton()->StopCue( m_iCue );

	AGameState* pState    = AGameStateController::GetSingleton()->GetCurrentState();
	m_iLastNumChildStates = pState->GetNumChildStates();

	if ( pState->GetClass()->IsA( &TGetClass( AMiniGameInstructionState ) ) == TTRUE )
	{
		m_iSound = TSTATICCAST( AMiniGameInstructionState, pState )->GetMiniGame()->GetSound();
	}
	else
	{
		m_iSound = m_iForcedSound;

		if ( m_iSound == -1 )
			m_iSound = pState->GetSound();
	}

	m_iLastDayPhase = AGameTimeManager::GetSingleton()->GetCurrentDayPhase();

	if ( m_iSound != -1 )
		m_iCue = ASoundManager::GetSingleton()->PlayCue( m_iSound );
}

// $Barnyard: FUNCTION 005d54d0
void AMusicManager::SetEnabled( TBOOL a_bEnable, TBOOL a_bCloseStream )
{
	TUINT16 uiFlags = AGameStateController::GetSingleton()->GetFlags();

	// Not during a transition
	if ( ( ( uiFlags & ( AGameStateController::FLAGS_IGNORE_INPUT | AGameStateController::FLAGS_LEAVE_IMMEDIATELY ) ) != 0 || ( uiFlags & AGameStateController::FLAGS_UNK10 ) != 0 ) && a_bEnable )
		return;

	if ( a_bEnable == TTRUE )
	{
		Restart();
	}
	else
	{
		ASoundManager* pSoundManager = ASoundManager::GetSingleton();

		// Remember where the music was to continue it once the start menu is closed
		if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass() == &TGetClass( AStartMenuState ) && m_iCue != -1 && pSoundManager->IsCueUsed( m_iCue ) == TTRUE )
		{
			m_iSavedStreamTime = pSoundManager->GetStreamCueTime( m_iCue );
		}
		else
		{
			m_iSavedStreamTime = -1;
		}

		pSoundManager->StopCue( m_iCue );

		if ( AGameLoader::IsMainGameLoaded() == TTRUE )
		{
			if ( AMultiplexStream::GetSingleton()->GetState() != AMultiplexStream::STATE_CLOSED )
				AMultiplexStream::GetSingleton()->Close( a_bCloseStream );

			m_strCurrentStream = TPString8();
			m_strMissionStream = TPString8();
			m_bFadingIn        = TFALSE;
			m_bFadingOut       = TFALSE;
			m_fFadeTime        = 0.0f;
		}

		m_pLastState = TNULL;
	}

	m_bEnabled = a_bEnable;
	ASoundManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );
	m_fFadeDuration = s_fDefaultFadeTime;
}

// $Barnyard: FUNCTION 005d46f0
void AMusicManager::ClearForcedSound( TBOOL a_bFadeOut )
{
	if ( m_iForcedSound == -1 )
		return;

	if ( a_bFadeOut == TTRUE )
	{
		ASoundManager::GetSingleton()->FadeOutCue( m_iCue, 1.0f, TTRUE, 3.0f );
		m_iCue   = -1;
		m_iSound = -1;
	}

	m_iForcedSound = -1;
	m_bForceUpdate = TTRUE;
}

// $Barnyard: FUNCTION 005d4740
TBOOL AMusicManager::IsMusicOver()
{
	return m_iCue == -1 || !ASoundManager::GetSingleton()->IsCueUsed( m_iCue );
}

// $Barnyard: FUNCTION 005d45d0
void AMusicManager::SetVolume( TFLOAT a_fVolume )
{
	m_fVolume = a_fVolume;
	TMath::Clip( m_fVolume, 0.0f, 1.0f );
}
