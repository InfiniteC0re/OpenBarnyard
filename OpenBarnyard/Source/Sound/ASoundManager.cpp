#include "pch.h"
#include "ASoundManager.h"
#include "APositionalSoundSet.h"
#include "ASoundLegacy.h"
#include "ASound.h"
#include "FSM/AMethodPointer.h"
#include "Cameras/ACameraManager.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetLoader.h"
#include "Assets/AAssetStreaming.h"
#include "Terrain/ATerrainInterface.h"
#include "Memory/AMemory.h"
#include "Units/AUnit.h"
#include "UnitMovement/Steering/ASteer.h"
#include "AWaveBankFMODFSB.h"
#include "AWaveBankFMODFSBStream.h"
#include "ALoadScreen.h"
#include "ARandom.h"
#include "Player/APlayerProfileManager.h"
#include "Quests/AQuestManager.h"
#include <Toshi/TScheduler.h>
#include <Render/TRenderInterface.h>

#include <Plugins/PPropertyParser/PBProperties.h>

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( am_m );
TPSTRING8_DECLARE( am_f );
TPSTRING8_DECLARE( VO );

// NOTE: GetClass, the create and the create-in-place functions of the TClass
// $Barnyard: FUNCTION 005d6190
// $Barnyard: FUNCTION 005db730
// $Barnyard: FUNCTION 005db790
TDEFINE_CLASS_INIT( ASoundManager );

// DAT_0078c480
AFunctionTable ASoundManager::ms_oFunctionTable( &TGetClass( ASoundManager ), TNULL );

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 005d64d0
static TObject* __stdcall ASoundManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return ASoundManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 005dbec0
void ASoundManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &ASoundManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1ArgsRT<TINT, ASoundManager, ASoundId>::Create( "Play", &ASoundManager::PlayCue ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2ArgsRT<TINT, ASoundManager, ASoundId, AUnit*>::Create( "Play3DFromUnit", &ASoundManager::PlayCueAtUnit ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<ASoundManager, TINT&>::Create( "Stop", &ASoundManager::StopCue ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<ASoundManager, TINT, TFLOAT>::Create( "SetVolume", &ASoundManager::SetCueVolume ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<ASoundManager, TINT, TBOOL>::Create( "SetPaused", &ASoundManager::PauseCue ) );
}

// $Barnyard: FUNCTION 005d61a0
void ASoundManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 005dab40
AFunctionTable* ASoundManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

#define MAX_NUM_SOUND_EVENTS 256
#define MAX_NUM_STREAMS      64

// $Barnyard: FUNCTION 005da7f0
ASoundManager::ASoundManager()
    : m_SoundEventPool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_SOUND_EVENTS ), m_StreamRefPool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_STREAMS ), m_ChannelRefPool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_SOUND_EVENTS ), m_ChannelRefLegacyPool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_SOUND_EVENTS ), m_CategoryIndices( AMemory::GetAllocator( AMemory::POOL_Sound ) ), m_SoundIdToSound( AMemory::GetAllocator( AMemory::POOL_Sound ) ), m_SoundIdToSoundLegacy( AMemory::GetAllocator( AMemory::POOL_Sound ) )
{
	m_iLastAvailableSoundExSlot = -1;
	m_fCurrentTime              = 0.0f;

	m_bMuted                  = TFALSE;
	m_bUseMinHardwareChannels = TTRUE;
	m_iMinHWChannels          = 32;
	m_iNumChannels            = 32;
	m_iGlobalFrequency        = 22050;

	ms_pFileSystem = TFileManager::GetSingleton()->FindFileSystem( "local" );

	// Set event handlers
	m_oEventManager.SetEventHandler( SOUNDEVENT_PlayAudio, &ASoundManager::EventHandler_PlaySound );
	m_oEventManager.SetEventHandler( SOUNDEVENT_PlayStream, &ASoundManager::EventHandler_PlayStream );
	m_oEventManager.SetEventHandler( SOUNDEVENT_StopAudio, &ASoundManager::EventHandler_StopAudio );
	m_oEventManager.SetEventHandler( SOUNDEVENT_UpdateChannelVolume, &ASoundManager::EventHandler_UpdateChannelVolume );
	m_oEventManager.SetEventHandler( SOUNDEVENT_UpdateChannelFrequency, &ASoundManager::EventHandler_UpdateChannelFrequency );
	m_oEventManager.SetEventHandler( SOUNDEVENT_UpdateChannelPosition, &ASoundManager::EventHandler_UpdatePosition );
}

// $Barnyard: FUNCTION 005dbf70
// $Barnyard: FUNCTION 005dc110
ASoundManager::~ASoundManager()
{
	Reset();

	m_CategoryIndices.Clear();
	m_FreeListS2.Clear();
	m_UsedListS2.Clear();

	for ( TUINT i = 0; i < MAX_NUM_CATEGORIES; i++ )
		m_aCategories[ i ].PlayingCues.Clear();

	AWaveBank::DestroyAll();

	FSOUND_Close();
	m_PauseListener.Disconnect();
}

// $Barnyard: FUNCTION 005dab50
TBOOL ASoundManager::Reset()
{
	if ( APositionalSoundSet::IsSingletonCreated() )
	{
		APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );
		APositionalSoundSet::GetSingleton()->StopSounds();
	}

	// Wait until all of the streams are over
	while ( !m_StreamRefs.IsEmpty() )
	{
		for ( auto pStream = m_StreamRefs.Begin(); pStream != m_StreamRefs.End(); )
		{
			if ( ( pStream->pLegacySlot && UpdateStreamActivity( pStream ) ) ||
			     ( pStream->pCue && UpdateStreamCue( pStream ) ) )
			{
				auto itNext = m_StreamRefs.Erase( pStream );
				m_StreamRefPool.DeleteObject( pStream );

				pStream = itNext;
			}
			else
			{
				pStream = pStream.Next();
			}
		}

		if ( m_StreamRefs.IsEmpty() )
			break;

		// Keep the game alive while waiting
		TRenderInterface::GetSingleton()->Update( 0.01f );
		AAssetStreaming::GetSingleton()->Update();
		ThreadSleep( 5 );

		if ( ATerrainInterface::IsSingletonCreated() )
			ATerrainInterface::GetSingleton()->Update();

		if ( g_oLoadScreen.IsLoadingScreen() )
			g_oLoadScreen.Update( 1.0f, TTRUE );
	}

	StopAllCues();

	// Free events of every queued list.
	// NOTE: the lists themselves stay linked here, Cue::Reset below unlinks them
	// (every queued list is the oEventList of some cue)
	T2_FOREACH( m_QueuedSortedEventLists, it )
	{
		SoundEventList::Type* pEventList = it->GetNodeValue();

		while ( !pEventList->IsEmpty() )
		{
			SoundEvent* pSoundEvent = pEventList->PopFront();
			m_SoundEventPool.DeleteObject( pSoundEvent );
		}
	}

	for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
	{
		m_aCues[ i ].Reset();
	}

	// Reset legacy sound slots
	for ( TINT i = 0; i < TARRAYSIZE( m_aS2 ); i++ )
	{
		S2*         pSlot       = &m_aS2[ i ];
		ChannelRef* pChannelRef = pSlot->pChannelRef;

		pSlot->fStartTime    = -1.0f;
		pSlot->fVolume       = -1.0f;
		pSlot->pSound        = TNULL;
		pSlot->bStreamQueued = TFALSE;

		if ( pChannelRef )
		{
			FSOUND_StopSound( pChannelRef->iFMODChannelHandle );
			pChannelRef->pLegacySample->pWaveBank->ResetWave( pChannelRef->pLegacySample->iId );

			ASoundManager::GetSingleton()->m_ChannelRefPool.DeleteObject( pChannelRef );
			pSlot->pChannelRef = TNULL;
		}
	}

	while ( !m_UsedListS2.IsEmpty() )
	{
		m_FreeListS2.PushBack( m_UsedListS2.PopBack() );
	}

	while ( !m_StreamRefs.IsEmpty() )
	{
		m_StreamRefPool.DeleteObject( m_StreamRefs.PopFront() );
	}

	m_fCurrentTime = 0.0f;

	while ( !m_UsedListS4.IsEmpty() )
	{
		m_FreeListS4.PushBack( m_UsedListS4.PopBack() );
	}

	if ( ACameraManager::IsSingletonCreated() )
		ASoundManager::GetSingleton()->OnUpdate( 1.0f / 30.0f );

	return TTRUE;
}

// $Barnyard: FUNCTION 005d9c70
void ASoundManager::StopAllCues()
{
	for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
	{
		Cue* pCue = &m_aCues[ i ];

		if ( pCue->IsUsed() )
		{
			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( channel->iFMODChannelHandle != -1 )
				{
					if ( FSOUND_GetReserved( channel->iFMODChannelHandle ) )
						FSOUND_SetReserved( channel->iFMODChannelHandle, TFALSE );

					FSOUND_StopSound( channel->iFMODChannelHandle );
				}

				channel->pSample->pWaveBank->ResetWave( channel->pSample->iId );
			}

			pCue->Reset();
		}
	}

	for ( auto it = m_StreamRefs.Begin(); it != m_StreamRefs.End(); it++ )
	{
		ASound::Sample* pSample       = it->pChannelRef->pSample;
		FSOUND_STREAM*  pStreamHandle = (FSOUND_STREAM*)pSample->pWaveBank->GetWaveSampleHandle( pSample->iId );

		FSOUND_Stream_Stop( pStreamHandle );
	}

	FSOUND_StopSound( FSOUND_ALL );
}

// $Barnyard: FUNCTION 005d7b90
TBOOL __stdcall ASoundManager::OnSystemPause( ASoundManager* a_pSoundManager, TSystemManager* a_pSystemManager, TBOOL* a_pPaused )
{
	a_pSoundManager->PauseAllCues( *a_pPaused );
	return TTRUE;
}

// $Barnyard: FUNCTION 005d8170
TBOOL ASoundManager::OnCreate()
{
	Initialise();
	m_PauseListener.Connect(
	    g_oSystemManager.GetPauseEmitter(),
	    this,
	    OnSystemPause,
	    0
	);

	m_pS4 = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) S4[ 32 ];

	for ( TINT i = 0; i < 32; i++ )
	{
		m_FreeListS4.PushBack( &m_pS4[ i ] );
	}

	for ( TINT i = 0; i < TARRAYSIZE( m_aS2 ); i++ )
	{
		m_FreeListS2.PushBack( &m_aS2[ i ] );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005db7d0
TBOOL ASoundManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	m_fCurrentTime += a_fDeltaTime;

	if ( !g_oSystemManager.IsPaused() )
	{
		if ( ARootTask::GetSingleton()->IsPaused() && ms_iNumPauses <= 0 )
		{
			PauseAllCues( TTRUE );
		}
		else if ( !ARootTask::GetSingleton()->IsPaused() && ms_iNumPauses > 0 )
		{
			PauseAllCues( TFALSE );
		}
	}

	UpdateFades( a_fDeltaTime );

	// Run the event loop
	while ( !m_QueuedSortedEventLists.IsEmpty() )
	{
		SoundEventList&            eventList      = *m_QueuedSortedEventLists.Front();
		ASoundManager::SoundEvent* pEarliestEvent = eventList->Front();

		// Skip the list if it's too early to execute it's earliest event
		if ( pEarliestEvent->fStartTime > m_fCurrentTime )
			break;

		// Handle events in the list
		while ( TTRUE )
		{
			pEarliestEvent->Remove();
			m_oEventManager.ExecuteEvent( pEarliestEvent->eEventType, this, pEarliestEvent );

			// Deallocate the event object
			m_SoundEventPool.DeleteObject( pEarliestEvent );

			if ( !eventList.IsLinked() )
				break;

			// Check if the list is over
			if ( eventList->IsEmpty() )
			{
				// Unlink the list since it's not used anymore
				eventList.Remove();
				break;
			}

			// Certainly, there are some more events
			// Let's check if they need to be processed
			pEarliestEvent = eventList->Front();

			if ( pEarliestEvent->fStartTime > m_fCurrentTime )
			{
				// Seems that the other events in the list are not to be executed yet,
				// so let's reinsert the list to preserve correct order and move on to
				// the next list.

				m_QueuedSortedEventLists.ReInsert( eventList );
				break;
			}
		}
	}

	// Update streams
	for ( auto pStream = m_StreamRefs.Begin(); pStream != m_StreamRefs.End(); )
	{
		if ( ( pStream->pLegacySlot && UpdateStreamActivity( pStream ) ) ||
		     ( pStream->pCue && UpdateStreamCue( pStream ) ) )
		{
			auto itNext = m_StreamRefs.Erase( pStream );
			m_StreamRefPool.DeleteObject( pStream );

			pStream = itNext;
		}
		else
		{
			pStream = pStream.Next();
		}
	}

	// Reset cues that has finished playing
	for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
	{
		TFLOAT fCueStartTime = m_aCues[ i ].fStartTime;

		if ( fCueStartTime > 0.0f && !IsCuePlaying( i ) )
		{
			m_aCues[ i ].Reset();
		}
	}

	// Free the slots of the legacy sounds that have finished playing
	for ( auto it = m_UsedListS2.Begin(); it != m_UsedListS2.End(); )
	{
		S2* pSlot = it;
		it        = it.Next();

		if ( pSlot->fVolume <= 0.0f && pSlot->fStartTime > 0.0f && pSlot->pSound )
		{
			ChannelRef* pChannel   = pSlot->pChannelRef;
			TBOOL       bIsPlaying = FSOUND_IsPlaying( pChannel->iFMODChannelHandle );

			if ( ( pChannel->iFlags & ASound::FLAGS_STARTED ) && !bIsPlaying )
			{
				pSlot->Reset();
				m_UsedListS2.Erase( pSlot );
				m_FreeListS2.PushBack( pSlot );
			}
		}
	}

	// Update sound attributes based on the current camera position
	// Not during the quest sequences
	if ( ARootTask::GetSingleton()->IsGameSystemCreated() && !AQuestManager::GetSingleton()->IsTimerPaused() )
	{
		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		// Get the position
		m_oCameraData.Position = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

		// Get the forward vector
		m_oCameraData.Forward   = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
		m_oCameraData.Forward.w = 1.0f;

		// Get the up vector
		m_oCameraData.Up   = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
		m_oCameraData.Up.w = 1.0f;

		// Invert up axis as it needs to be
		m_oCameraData.Up.x = -m_oCameraData.Up.x;
		m_oCameraData.Up.y = -m_oCameraData.Up.y;
		m_oCameraData.Up.z = -m_oCameraData.Up.z;

		TFLOAT aPosition[ 3 ];
		aPosition[ 0 ] = m_oCameraData.Position.x;
		aPosition[ 1 ] = m_oCameraData.Position.y;
		aPosition[ 2 ] = -m_oCameraData.Position.z;

		TFLOAT aVelocity[ 3 ];
		aVelocity[ 0 ] = m_oCameraData.Velocity.x;
		aVelocity[ 1 ] = m_oCameraData.Velocity.y;
		aVelocity[ 2 ] = -m_oCameraData.Velocity.z;

		FSOUND_3D_Listener_SetAttributes( aPosition, aVelocity, m_oCameraData.Forward.x, m_oCameraData.Forward.y, -m_oCameraData.Forward.z, m_oCameraData.Up.x, m_oCameraData.Up.y, -m_oCameraData.Up.z );
	}

	FSOUND_Update();
	return TTRUE;
}

// $Barnyard: FUNCTION 005d7290
void ASoundManager::OnDestroy()
{
	m_UsedListS4.Clear();
	m_FreeListS4.Clear();
	delete[] m_pS4;
	m_pS4 = TNULL;
	m_PauseListener.Disconnect();
}

// $Barnyard: FUNCTION 005dade0
TINT ASoundManager::PlayCue( ASoundId a_iSound )
{
	return PlayCueEx( a_iSound, 1.0f, TFALSE, 0.0f, -1 );
}

// $Barnyard: FUNCTION 005d9980
TINT ASoundManager::PlayCueEx( ASoundId a_iSound, TFLOAT a_fVolume, TBOOL a_bFlag, TFLOAT a_fDelay, TINT a_iTrack )
{
	if ( a_iSound == ASOUNDID_INVALID )
		return -1;

	// Look if the sound is stored to be played
	auto pFoundNode = m_SoundIdToSound.Find( a_iSound );

	if ( !m_SoundIdToSound.IsValid( pFoundNode ) )
		return -1;

	// The sound is found, can use it
	ASound* pSound = pFoundNode->GetSecond();

	// Check if no waves are stored in the sound
	if ( pSound->m_vecSamples.Size() == 0 )
		return -1;

	TINT iCueIndex = GetAvailableCueIndex();

	// Check if we can use some cue to play the sound
	if ( iCueIndex == -1 )
		return -1;

	Cue* pCue        = &m_aCues[ iCueIndex ];
	pCue->fStartTime = m_fCurrentTime;

	// Add this cue to a category
	m_aCategories[ pSound->m_uiCategoryIndex ].PlayingCues.PushBack( pCue );

	TINT iFlags = pSound->m_iFlags;

	if ( a_bFlag )
	{
		iFlags |= ASound::FLAGS_PAUSED;
		pCue->fStartTime2 = m_fCurrentTime;
	}

	// Setup basic settings on the cue
	pCue->fVolume    = ( a_fVolume > 1.0f ) ? 1.0f : a_fVolume;
	pCue->pSound     = pSound;
	pCue->fFrequency = 1.0f;
	pCue->vecLoopStarts.Copy( pSound->m_vecLoopStarts );

	TINT iSoundNumTracks = pSound->m_vecTracks.Size();

	if ( a_iTrack == -1 )
	{
		// Play all the tracks of this sound

		if ( iSoundNumTracks > 0 )
		{
			TINT iFirstWaveIndex = 0;

			for ( TINT iTrack = 0; iTrack < iSoundNumTracks; iTrack++ )
			{
				if ( pSound->m_TrackLoop.GetBit( iTrack ) != 0 )
					iFlags |= ASound::FLAGS_LOOP;
				else
					iFlags &= ~ASound::FLAGS_LOOP;

				CreatePlaySoundEvent(
				    pCue,
				    iTrack,
				    iFirstWaveIndex,
				    pSound->m_vecTracks[ iTrack ] + iFirstWaveIndex,
				    iFlags,
				    a_fDelay,
				    -1.0f
				);

				iFirstWaveIndex += pSound->m_vecTracks[ iTrack ];
			}
		}
	}
	else
	{
		// Play only the specified track

		TINT iFirstWaveIndex = pSound->GetFirstWaveForTrack( a_iTrack );

		if ( pSound->m_TrackLoop.GetBit( a_iTrack ) != 0 )
			iFlags |= ASound::FLAGS_LOOP;
		else
			iFlags &= ~ASound::FLAGS_LOOP;

		CreatePlaySoundEvent(
		    pCue,
		    a_iTrack,
		    iFirstWaveIndex,
		    pSound->m_vecTracks[ a_iTrack ] + iFirstWaveIndex,
		    iFlags,
		    a_fDelay,
		    -1.0f
		);
	}

	pCue->bUsed = TTRUE;

	return iCueIndex;
}

// $Barnyard: FUNCTION 005da000
void ASoundManager::SetCueFrequency( TINT a_iCueIndex, TFLOAT a_fFrequency, TFLOAT a_fDelay )
{
	if ( a_iCueIndex == -1 )
		return;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( pCue->IsUsed() && pCue->fFrequency != a_fFrequency )
	{
		pCue->fFrequency = a_fFrequency;

		T2_FOREACH( pCue->oChannelRefs, channel )
		{
			if ( channel->iFMODChannelHandle == -1 )
				channel->iFrequency = TINT( channel->pSample->iFrequency * a_fFrequency );
			else
				CreateSoundEvent( SOUNDEVENT_UpdateChannelFrequency, a_fDelay, pCue, channel->pSample, 1.0f, channel, 0, -1 );
		}
	}
}

// $Barnyard: FUNCTION 005daf50
void ASoundManager::SetCuePositionToUnit( TINT a_iCueIndex, AUnit* a_pUnit )
{
	TVector4 vecPosition = *a_pUnit->GetSteer()->GetPosition();
	SetCuePosition( a_iCueIndex, vecPosition, 0.0f );
}

// $Barnyard: FUNCTION 005dbb70
TINT ASoundManager::PlayCueAtUnit( ASoundId a_iSound, AUnit* a_pUnit )
{
	TVector4 vecPosition = *a_pUnit->GetSteer()->GetPosition();
	TINT     iCueIndex   = PlayCueEx( a_iSound, 1.0f, TFALSE, 0.0f, -1 );

	if ( iCueIndex != -1 )
	{
		SetCuePosition( iCueIndex, vecPosition, 0.0f );
		return iCueIndex;
	}

	return iCueIndex;
}

// $Barnyard: FUNCTION 005d7370
void ASoundManager::AddCueCallback( TINT a_iCueIndex, TINT a_iType, ChannelRefLegacy::Callback_t a_fnCallback, void* a_pUserData )
{
	if ( a_iCueIndex == -1 )
		return;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( pCue->fStartTime > 0.0f && m_ChannelRefLegacyPool.CanAllocate() )
	{
		ChannelRefLegacy* pChannelRef = m_ChannelRefLegacyPool.NewObject();
		pChannelRef->fnCallback       = a_fnCallback;
		pChannelRef->pUserData        = a_pUserData;
		pCue->aChannelRefsLegacy[ a_iType ].PushBack( pChannelRef );
	}
}

// $Barnyard: FUNCTION 005d7950
void ASoundManager::RemoveCueCallback( TINT a_iCueIndex, TINT a_iType, ChannelRefLegacy::Callback_t a_fnCallback )
{
	if ( a_iCueIndex == -1 )
		return;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( !pCue->bUsed )
		return;

	T2_FOREACH( pCue->aChannelRefsLegacy[ a_iType ], it )
	{
		if ( it->fnCallback == a_fnCallback )
		{
			// NOTE: the original only unlinks the node, it goes back to the pool when the cue is reset
			pCue->aChannelRefsLegacy[ a_iType ].Erase( it );
			return;
		}
	}
}

// $Barnyard: FUNCTION 005dae50
void ASoundManager::StopCue( TINT& a_rCueIndex )
{
	StopCueAsync( a_rCueIndex, 0.0f );
}

// $Barnyard: FUNCTION 005d66a0
TINT ASoundManager::GetAvailableCueIndex()
{
	TINT iSlot;

	for ( iSlot = m_iLastAvailableSoundExSlot + 1;; iSlot++ )
	{
		if ( iSlot == 128 )
			iSlot = 0;

		if ( iSlot == m_iLastAvailableSoundExSlot )
			return -1;

		if ( m_aCues[ iSlot ].fStartTime <= 0.0 )
		{
			if ( iSlot == -1 )
				return iSlot;

			if ( !m_aCues[ iSlot ].bUsed )
				break;
		}
	}

	m_iLastAvailableSoundExSlot = iSlot;
	return iSlot;
}

// $Barnyard: FUNCTION 005d7640
TBOOL ASoundManager::IsCuePlaying( TINT a_iCueIndex )
{
	TASSERT( a_iCueIndex < ASOUNDMANAGER_MAX_NUM_CUE );

	if ( a_iCueIndex == -1 )
		return TFALSE;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( pCue->fStartTime2 > 0.0f || !pCue->oEventList->IsEmpty() )
		return TTRUE;

	if ( pCue->IsUsed() && pCue->pSound != TNULL )
	{
		ASound* pSound = pCue->pSound;

		if ( pSound->m_iFlags & ASound::FLAGS_STREAM )
		{
			// Stream
			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( !( channel->iFlags & ASound::FLAGS_STARTED ) || FSOUND_IsPlaying( channel->iFMODChannelHandle ) )
				{
					return TTRUE;
				}
			}
		}
		else
		{
			// Loaded data
			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( FSOUND_IsPlaying( channel->iFMODChannelHandle ) )
				{
					return TTRUE;
				}
			}
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005d67f0
TBOOL ASoundManager::IsCueStreamReady( TINT a_iCueIndex ) const
{
	if ( a_iCueIndex == -1 )
		return TFALSE;

	const Cue* pCue = &m_aCues[ a_iCueIndex ];
	return pCue->bUsed && pCue->pSound && !( pCue->pSound->m_iFlags & ASound::FLAGS_STREAM_OPENING );
}

// $Barnyard: FUNCTION 005d77d0
TBOOL ASoundManager::IsCueFading( TINT a_iCueIndex )
{
	T2_FOREACH( m_UsedListS4, it )
	{
		if ( it->iCueIndex == a_iCueIndex )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005d7f60
TBOOL ASoundManager::IsStreamCueActive( TINT a_iCueIndex )
{
	if ( a_iCueIndex == -1 )
		return TFALSE;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( !pCue->bUsed || !pCue->pSound || !( pCue->pSound->m_iFlags & ASound::FLAGS_STARTED ) )
		return TFALSE;

	if ( IsCueUsed( a_iCueIndex ) == TTRUE || IsCueFading( a_iCueIndex ) == TTRUE )
		return TTRUE;

	T2_FOREACH( pCue->oChannelRefs, channel )
	{
		if ( channel->iFMODChannelHandle != -1 )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005d7820
TINT ASoundManager::GetStreamCueTime( TINT a_iCueIndex )
{
	if ( a_iCueIndex == -1 )
		return -1;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( !pCue->bUsed || !pCue->pSound )
		return -1;

	// Has to be an opened stream that is playing
	TINT iFlags = pCue->pSound->m_iFlags;
	if ( !( iFlags & ASound::FLAGS_STARTED ) || !( iFlags & ASound::FLAGS_STREAM ) || ( iFlags & ASound::FLAGS_STREAM_OPENING ) )
		return -1;

	ASound::Sample* pSample = pCue->oChannelRefs.Begin()->pSample;
	return FSOUND_Stream_GetTime( (FSOUND_STREAM*)pSample->pWaveBank->GetWaveSampleHandle( pSample->iId ) );
}

// $Barnyard: FUNCTION 005d7890
void ASoundManager::SetStreamCueTime( TINT a_iCueIndex, TINT a_iTime )
{
	if ( a_iCueIndex == -1 )
		return;

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( !pCue->bUsed || !pCue->pSound )
		return;

	// Has to be an opened stream that is playing
	TINT iFlags = pCue->pSound->m_iFlags;
	if ( !( iFlags & ASound::FLAGS_STARTED ) || !( iFlags & ASound::FLAGS_STREAM ) || ( iFlags & ASound::FLAGS_STREAM_OPENING ) )
		return;

	ASound::Sample* pSample = pCue->oChannelRefs.Begin()->pSample;
	FSOUND_Stream_SetTime( (FSOUND_STREAM*)pSample->pWaveBank->GetWaveSampleHandle( pSample->iId ), a_iTime );
}

// $Barnyard: FUNCTION 005d7ff0
TINT ASoundManager::PlayStreamedVO( ASoundId a_iSound )
{
	if ( a_iSound == ASOUNDID_INVALID )
		return -1;

	auto pFoundNode = m_SoundIdToSoundLegacy.Find( a_iSound );

	if ( !m_SoundIdToSoundLegacy.IsValid( pFoundNode ) )
		return -1;

	ASoundLegacy* pSound = pFoundNode->GetSecond();

	// NOTE: retail reads the sample handle of the wave here and doesn't use it
	pSound->m_pWaveBank->GetWaveSampleHandle( pSound->m_iWaveId );

	// NOTE: retail doesn't check if there's a free slot
	S2* pSlot         = m_FreeListS2.PopBack();
	pSlot->fStartTime = m_fCurrentTime;
	m_UsedListS2.PushBack( pSlot );
	pSlot->pSound       = pSound;
	pSlot->fVolumeScale = 1.0f;

	// NOTE: retail keeps the slot in the used list in this case
	if ( !m_ChannelRefPool.CanAllocate() )
		return -1;

	ChannelRef* pChannelRef    = m_ChannelRefPool.NewObject();
	pSlot->pChannelRef         = pChannelRef;
	pChannelRef->pLegacySample = TREINTERPRETCAST( ASound::Sample*, &pSound->m_pWaveBank );
	pChannelRef->fVolume       = GetCategoryVolume( TPS8( VO ) ) * pSlot->fVolumeScale;
	TMath::Clip( pChannelRef->fVolume, 0.0f, 1.0f );
	pChannelRef->iFrequency = pChannelRef->pLegacySample->pWaveBank->GetWaveFrequency( pChannelRef->pLegacySample->iId );
	pChannelRef->iFlags     = pSound->m_iFlags & ( ~ASound::FLAGS_STARTED );

	if ( m_StreamRefPool.CanAllocate() )
	{
		StreamRef* pStreamRef           = m_StreamRefPool.NewObject();
		pChannelRef->iFMODChannelHandle = -1;
		pStreamRef->pChannelRef         = pChannelRef;
		pStreamRef->pLegacySlot         = pSlot;
		m_StreamRefs.PushBack( pStreamRef );
		pSlot->bStreamQueued = TTRUE;
	}

	for ( TINT i = 0; i < TARRAYSIZE( m_aS2 ); i++ )
	{
		if ( &m_aS2[ i ] == pSlot )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 005d69d0
TBOOL ASoundManager::IsStreamedVOQueued( TINT a_iSlot ) const
{
	if ( a_iSlot == -1 )
		return TFALSE;

	return m_aS2[ a_iSlot ].bStreamQueued;
}

// $Barnyard: FUNCTION 005d6980
void ASoundManager::SetStreamedVOPosition( TINT a_iSlot, const TVector4& a_rcPosition )
{
	if ( a_iSlot == -1 )
		return;

	TFLOAT afPosition[ 3 ] = { a_rcPosition.x, a_rcPosition.y, -a_rcPosition.z };
	FSOUND_3D_SetAttributes( m_aS2[ a_iSlot ].pChannelRef->iFMODChannelHandle, afPosition, TNULL );
}

// $Barnyard: FUNCTION 005daee0
void ASoundManager::SetAllCuesVolume( TFLOAT a_fVolume )
{
	for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
		SetCueVolume( i, a_fVolume, 0.0f );
}

// $Barnyard: FUNCTION 005d9b80
void ASoundManager::StopCueAsync( TINT& a_rCueIndex, TFLOAT a_fDelay )
{
	if ( a_rCueIndex == -1 )
		return;

	TASSERT( a_rCueIndex >= 0 && a_rCueIndex < ASOUNDMANAGER_MAX_NUM_CUE );
	Cue* pCue   = &m_aCues[ a_rCueIndex ];
	a_rCueIndex = -1;

	if ( pCue->IsUsed() )
	{
		if ( pCue->oChannelRefs.Size() == 0 && a_fDelay == 0.0f )
		{
			// No need to remove events or wait for async stop
			pCue->Reset();
		}
		else if ( !pCue->HasEventOfType( SOUNDEVENT_StopAudio ) )
		{
			CancelCueEvents( pCue, SOUNDEVENT_PlayAudio );

			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( channel->iFMODChannelHandle == -1 )
					continue;

				// Stop audio for every valid channel
				CreateSoundEvent( SOUNDEVENT_StopAudio, a_fDelay, pCue, channel->pSample, channel, 0, -1 );
			}
		}
	}
}

// $Barnyard: FUNCTION 005d78f0
void ASoundManager::CancelCueEvents( Cue* a_pCue, SOUNDEVENT a_eEventType )
{
	TVALIDPTR( a_pCue );

	for ( auto it = a_pCue->oEventList->Begin(); it != a_pCue->oEventList->End(); )
	{
		auto itNext = it.Next();

		if ( it->eEventType == a_eEventType )
		{
			a_pCue->oEventList->Erase( it );
			m_SoundEventPool.DeleteObject( it );
		}

		it = itNext;
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original leaves the cue's event list queued even when it erased every event
	// (or the earliest one). An empty queued list makes OnUpdate run the list's sentinel as an
	// event (IsLinked assert, invalid event type, access violation). Unlink empty lists and
	// re-sort the others.
	if ( a_pCue->oEventList.IsLinked() )
	{
		if ( a_pCue->oEventList->IsEmpty() )
			a_pCue->oEventList.Remove();
		else
			m_QueuedSortedEventLists.ReInsert( a_pCue->oEventList );
	}
#endif
}

// $Barnyard: FUNCTION 005d9d80
void ASoundManager::SetCueVolume( TINT a_iCueIndex, TFLOAT a_fVolume, TFLOAT a_fDelay )
{
	if ( a_iCueIndex == -1 )
		return;

	TFLOAT fVolume = ( a_fVolume <= 0.0f ) ? 0.0f : a_fVolume;
	Cue*   pCue    = &m_aCues[ a_iCueIndex ];

	if ( pCue->IsUsed() )
	{
		if ( fVolume > 1.0f )
			fVolume = 1.0f;

		T2_FOREACH( pCue->oChannelRefs, channel )
		{
			if ( channel->iFMODChannelHandle == -1 )
				pCue->fVolume = fVolume;
			else
				CreateSoundEvent( SOUNDEVENT_UpdateChannelVolume, a_fDelay, pCue, channel->pSample, fVolume, channel, 0, -1 );
		}
	}
}

// NOTE: inlined into FadeCue and FadeOutCue
void ASoundManager::RemoveCueFades( TINT a_iCueIndex )
{
	for ( auto it = m_UsedListS4.Begin(); it != m_UsedListS4.End(); )
	{
		auto itNext = it.Next();

		if ( it->iCueIndex == a_iCueIndex )
		{
			it->Remove();
			m_FreeListS4.PushBack( it );
		}

		it = itNext;
	}
}

// $Barnyard: FUNCTION 005db0b0
void ASoundManager::FadeCue( TINT a_iCueIndex, TFLOAT a_fDuration, TFLOAT a_fVolume, TFLOAT a_fUnused )
{
	if ( a_iCueIndex == -1 )
		return;

	Cue* pCue = &m_aCues[ a_iCueIndex ];
	RemoveCueFades( a_iCueIndex );

	if ( m_FreeListS4.IsEmpty() )
	{
		// No fade left, set the volume right away
		SetCueVolume( a_iCueIndex, a_fVolume, 0.0f );
	}
	else
	{
		S4* pFade = m_FreeListS4.PopBack();

		pFade->iCueIndex     = a_iCueIndex;
		pFade->fTime         = 0.0f;
		pFade->fUpdateTimer  = 0.0f;
		pFade->bStopWhenDone = TFALSE;
		pFade->fDuration     = a_fDuration;
		pFade->fEndVolume    = a_fVolume;
		pFade->fStartVolume  = pCue->fVolume;

		m_UsedListS4.PushBack( pFade );
	}
}

// $Barnyard: FUNCTION 005dafa0
void ASoundManager::FadeOutCue( TINT a_iCueIndex, TFLOAT a_fDuration, TBOOL a_bStop, TFLOAT a_fUnused )
{
	if ( a_iCueIndex == -1 )
		return;

	Cue* pCue = &m_aCues[ a_iCueIndex ];
	RemoveCueFades( a_iCueIndex );

	if ( m_FreeListS4.IsEmpty() )
	{
		// No fade left, stop or mute the cue right away
		if ( a_bStop == TTRUE )
			StopCueAsync( a_iCueIndex, 0.0f );
		else
			SetCueVolume( a_iCueIndex, 0.0f, 0.0f );
	}
	else
	{
		S4* pFade = m_FreeListS4.PopBack();

		pFade->iCueIndex     = a_iCueIndex;
		pFade->fTime         = 0.0f;
		pFade->fUpdateTimer  = 0.0f;
		pFade->fEndVolume    = 0.0f;
		pFade->fDuration     = a_fDuration;
		pFade->bStopWhenDone = a_bStop;
		pFade->fStartVolume  = pCue->fVolume;

		m_UsedListS4.PushBack( pFade );
	}
}

// $Barnyard: FUNCTION 005db310
void ASoundManager::UpdateFades( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_UsedListS4.Begin(); it != m_UsedListS4.End(); )
	{
		auto itNext = it.Next();
		S4*  pFade  = it;
		it          = itNext;

		const TINT iCueIndex = pFade->iCueIndex;

		// Wait while the cue is delayed
		if ( iCueIndex != -1 && m_aCues[ iCueIndex ].pSound && m_aCues[ iCueIndex ].fStartTime2 > 0.0f )
			continue;

		pFade->fTime += a_fDeltaTime;
		pFade->fUpdateTimer += a_fDeltaTime;

		TBOOL bDone = TFALSE;

		if ( iCueIndex == -1 || !m_aCues[ iCueIndex ].bUsed )
		{
			bDone = TTRUE;
		}
		else if ( pFade->fTime >= pFade->fDuration )
		{
			if ( pFade->bStopWhenDone == TTRUE )
			{
				Cue* pCue = &m_aCues[ iCueIndex ];
				CancelCueEvents( pCue, SOUNDEVENT_PlayAudio );
				CancelCueEvents( pCue, SOUNDEVENT_UpdateChannelVolume );
				StopCueAsync( pFade->iCueIndex, 0.0f );
			}

			bDone = TTRUE;
		}
		else if ( pFade->fUpdateTimer >= 0.1f )
		{
			pFade->fUpdateTimer = 0.0f;

			// NOTE: the original computes this and doesn't use it
			TFLOAT fProgress = pFade->fTime / pFade->fDuration;
			TMath::Clip( fProgress, 0.0f, 1.0f );

			SetCueVolume( iCueIndex, ( pFade->fEndVolume - pFade->fStartVolume ) / pFade->fDuration * pFade->fTime + pFade->fStartVolume, 0.0f );
		}

		if ( bDone )
		{
			pFade->fTime         = 0.0f;
			pFade->fUpdateTimer  = 0.0f;
			pFade->iCueIndex     = -1;
			pFade->fEndVolume    = 0.0f;
			pFade->fDuration     = 0.0f;
			pFade->bStopWhenDone = TFALSE;

			pFade->Remove();
			m_FreeListS4.PushBack( pFade );
		}
	}
}

// $Barnyard: FUNCTION 005dbc30
void ASoundManager::SetCategoryVolume( const TPString8& a_rcName, TFLOAT a_fVolume, TFLOAT a_fDelay )
{
	auto pFoundCategory = m_CategoryIndices.Find( a_rcName );
	TINT iCategory;

	if ( m_CategoryIndices.IsValid( pFoundCategory ) )
	{
		iCategory = TINT( pFoundCategory->GetSecond() );
	}
	else
	{
		// An unknown category gets the next index, like LoadWaveBanksInfo does. This happens when the save
		// is loaded at boot (SaveLoad::ApplyLoadedSave -> AOptions::ApplySettings) before the wave bank info is loaded
		iCategory = TINT( m_CategoryIndices.Size() );
		m_CategoryIndices.Insert( a_rcName, TSIZE( iCategory ) );
	}

	SetCategoryVolume( iCategory, a_fVolume, a_fDelay );
}

// $Barnyard: FUNCTION 005d9e60
void ASoundManager::SetCategoryVolume( TINT a_iCategory, TFLOAT a_fVolume, TFLOAT a_fDelay )
{
	if ( a_iCategory == -1 )
		return;

	Category& rCategory             = m_aCategories[ a_iCategory ];
	rCategory.fPrevVolumeMultiplier = rCategory.fVolumeMultiplier;
	rCategory.fVolumeMultiplier     = a_fVolume;

	SoundEvent oEvent;

	T2_FOREACH( rCategory.PlayingCues, pCue )
	{
		TINT iCueIndex = GetCueIndex( pCue );

		if ( iCueIndex == -1 || !m_aCues[ iCueIndex ].pSound || m_aCues[ iCueIndex ].fStartTime2 <= 0.0f )
		{
			// Will be applied when the volume is set
			SetCueVolume( iCueIndex, GetCueVolume( iCueIndex ), a_fDelay );
		}
		else
		{
			// Update the volume of the channels right away
			T2_FOREACH( pCue->oChannelRefs, pChannel )
			{
				oEvent.eEventType       = SOUNDEVENT_UpdateChannelVolume;
				oEvent.fStartTime       = 0.0f;
				oEvent.pCue             = pCue;
				oEvent.pSample          = pChannel->pSample;
				oEvent.pChannel         = pChannel;
				oEvent.oParameters[ 0 ] = GetCueVolume( iCueIndex );
				EventHandler_UpdateChannelVolume( &oEvent );
			}
		}
	}
}

// $Barnyard: FUNCTION 005d74e0
TFLOAT ASoundManager::GetCategoryVolume( const TPString8& a_rcName )
{
	auto pFoundCategory = m_CategoryIndices.Find( a_rcName );

	if ( !m_CategoryIndices.IsValid( pFoundCategory ) )
		return 0.0f;

	return m_aCategories[ pFoundCategory->GetSecond() ].fVolumeMultiplier;
}

// $Barnyard: FUNCTION 005d73d0
void ASoundManager::SetGlobalVolume( TFLOAT a_flVolume )
{
	for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
	{
		Cue*         pCue           = &m_aCues[ i ];
		const TFLOAT flCueStartTime = pCue->fStartTime;
		const TBOOL  bIsPlaying     = flCueStartTime > 0.0f;

		// Calculate volume for this exact Cue
		TFLOAT flGlobalVolume = ( bIsPlaying ? pCue->fVolume : 0.0f ) * a_flVolume;

		if ( bIsPlaying )
		{
			// Set volume for each channel
			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				// Skip invalid channels and sounds
				if ( channel->iFMODChannelHandle == -1 || pCue->pSound == TNULL ) continue;

				// Obtain volume for this category of sounds
				const TFLOAT flCategoryVolume = m_aCategories[ pCue->pSound->m_uiCategoryIndex ].fVolumeMultiplier;
				const TFLOAT flSampleVolume   = channel->pSample->fVolume;

				// Calculate actual volume for this channel
				TFLOAT flActualVolume = flCategoryVolume * flSampleVolume * flGlobalVolume;
				TMath::Clip( flActualVolume, 0.0f, 1.0f );

				FSOUND_SetVolume( channel->iFMODChannelHandle, m_bMuted ? 0 : TINT( flActualVolume * 255.0f ) );
			}
		}
	}
}

// $Barnyard: FUNCTION 005dc350
AWaveBank* ASoundManager::FindWaveBank( const TPString8& a_rcName )
{
	auto pFoundNode = ms_WaveBanks.Find( a_rcName );

	return ( ms_WaveBanks.IsValid( pFoundNode ) ) ? pFoundNode->GetSecond() : TNULL;
}

// $Barnyard: FUNCTION 005dc810
AWaveBank* ASoundManager::LoadWaveBankFromAsset( const Toshi::TString8& a_strName, TUINT a_uiForcedFlags )
{
	// Generate bank file name
	TString8 strBankFileName = "Bank_";
	strBankFileName += a_strName;
	strBankFileName.MakeLower();

	TPString8 pStrBank( "Bank" );

	// Get PProperties symbol of the currently loaded sound asset file
	const PBProperties* pProperties =
	    AAssetLoader::CastSymbol<const PBProperties>( strBankFileName, PBProperties::TRB_SECTION_NAME, AAssetType_WaveBank );
	TVALIDPTR( pProperties );

	const PBProperties* pBankProperties = pProperties->Begin()->GetValue()->GetProperties();

	// Get the bank name
	TPString8 strWaveWankBankName;
	pBankProperties->GetOptionalPropertyValue( strWaveWankBankName, "name" );

	TINT iNameCmpRslt        = strWaveWankBankName.GetString8().Compare( a_strName, -1 );
	auto pExistingWaveBank   = ms_WaveBanks.Find( strWaveWankBankName );
	auto pWaveBankVersionVal = pBankProperties->GetProperty( "version" );

	// Store wavebank library
	auto      pWaveBankLibraryVal = pBankProperties->GetProperty( "library" );
	TPString8 strWaveBankLibrary  = ( pWaveBankLibraryVal ) ? pWaveBankLibraryVal->GetTPString8() : TPString8();

	// Store wavebank type
	auto      pWaveBankTypeVal = pBankProperties->GetProperty( "type" );
	TPString8 strWaveBankType  = ( pWaveBankTypeVal ) ? pWaveBankTypeVal->GetTPString8() : TPString8();

	// Store wavebank path
	auto      pWaveBankPathVal = pBankProperties->GetProperty( "path" );
	TPString8 strWaveBankPath  = ( pWaveBankPathVal ) ? pWaveBankPathVal->GetTPString8() : TPString8();

	// Store wavebank extension
	auto      pWaveBankExtensionVal = pBankProperties->GetProperty( "extension" );
	TPString8 strWaveBankExtension  = ( pWaveBankExtensionVal ) ? pWaveBankExtensionVal->GetTPString8() : TPString8();

	// Create the actual wavebank from the parameters
	AWaveBank* pWaveBank = AllocateWaveBank( strWaveWankBankName, strWaveBankLibrary, strWaveBankType, strWaveBankPath );

	pWaveBank->ParseWavesData( pBankProperties, a_uiForcedFlags );

	return pWaveBank;
}

// $Barnyard: FUNCTION 005dc5b0
AWaveBank* ASoundManager::AllocateWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strLibrary, const Toshi::TPString8& a_strType, const Toshi::TPString8& a_strPath )
{
	// Check if we are loading the supported format
	if ( a_strLibrary != TPString8( "FMOD" ) || a_strType == TPString8( "Dir" ) )
	{
		TASSERT( !"Trying to load an unknown non-FSB WaveBank format!" );
		return TNULL;
	}

	if ( a_strType == TPString8( "File" ) )
	{
		// Load from a file
		return new AWaveBankFMODFSB( a_strBank, a_strPath );
	}
	else if ( a_strType == TPString8( "Stream" ) )
	{
		// Load from a stream
		return new AWaveBankFMODFSBStream( a_strBank, a_strPath );
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005d8b30
TBOOL ASoundManager::LoadSoundBankImpl( const TCHAR* a_szName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately )
{
	g_oLoadScreen.Update();

	TString8 strFileName = SOUNDS_BASE_DIRECTORY;
	strFileName += "soundbank_";
	strFileName += a_szName;
	strFileName += ".trb";

	TTRB        trb;
	TTRB::ERROR eTrbLoadResult = trb.Load( strFileName );

	if ( eTrbLoadResult != TTRB::ERROR_OK )
	{
		TERROR( "Unable to load soundbank: %s\n", strFileName.GetString() );
		return TFALSE;
	}

	auto& rcProperties = *PBProperties::LoadFromTRB( trb );

	// Delete symbol table for some small optimisation
	trb.DeleteSymbolTable();

	TBOOL bNoErrors = TTRUE;

	T2_FOREACH( rcProperties, soundbank )
	{
		const PBProperties* pSoundsProperties = soundbank->GetValue()->GetProperties();

		TINT  iNumSounds;
		TBOOL bHasNumSounds = pSoundsProperties->GetOptionalPropertyValue( iNumSounds, "numsounds" );

		if ( !bHasNumSounds )
		{
			TASSERT( !"Invalid format of the soundbank property" );
			return TFALSE;
		}

		if ( iNumSounds == 0 )
		{
			return TTRUE;
		}

		// Allocate soundbank
		ASoundBank* pSoundBank = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASoundBank( iNumSounds );
		m_SoundBanks.PushBack( pSoundBank );

		if ( a_bSimpleSound )
		{
			pSoundBank->m_pSounds = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASoundLegacy[ iNumSounds ];
		}
		else
		{
			pSoundBank->m_pSoundsEx = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASound[ iNumSounds ];
		}

		pSoundsProperties->GetOptionalPropertyValue( pSoundBank->m_strName, "name" );

		TINT iLocalised;
		pSoundsProperties->GetOptionalPropertyValue( iLocalised, "localised" );

		TINT iSoundIndex = 0;
		T2_FOREACH( *pSoundsProperties, it )
		{
			// Skip anything that doesn't even look like 'Sound' property
			if ( it->GetName().GetString()[ 0 ] != 'S' )
				continue;

			auto pSoundProperties = it->GetValue()->GetProperties();

			TINT iSoundId;
			pSoundProperties->GetOptionalPropertyValue( iSoundId, "id" );

			if ( a_bSimpleSound )
			{
				// Initialise ASound
				ASoundLegacy* pSound = &pSoundBank->m_pSounds[ iSoundIndex++ ];

				pSound->m_iId = iSoundId;
				m_SoundIdToSoundLegacy.Insert( iSoundId, pSound );

				if ( !pSoundProperties->GetOptionalPropertyValue( pSound->m_iFlags, "flags" ) )
				{
					bNoErrors = TFALSE;
					TASSERT( !"No flags property specified in the soundbank" );
				}

				PBPropertyValueArray* pBanks = TNULL;
				if ( !pSoundProperties->GetOptionalPropertyValue( pBanks, "banks" ) )
				{
					bNoErrors = TFALSE;
					TASSERT( !"No banks property specified in the soundbank" );
					continue;
				}

				PBPropertyValueArray* pWaves;
				if ( bNoErrors && pSoundProperties->GetOptionalPropertyValue( pWaves, "waves" ) )
				{
					TASSERT( pBanks->GetSize() >= 1 );
					TASSERT( pWaves->GetSize() >= 1 );

					TPString8  strSoundBankName = pBanks->GetValue( 0 )->GetTPString8();
					AWaveBank* pWaveBank        = FindWaveBank( strSoundBankName );

					if ( a_bLoadImmediately )
					{
						LoadWaveBankSamples( strSoundBankName, iLocalised != 0 ? AWaveBank::LOADFLAGS_LOCALISE : AWaveBank::LOADFLAGS_NONE, -1 );
					}

					pSound->m_pWaveBank = pWaveBank;
					pSound->m_iWaveId   = pWaves->GetValue( 0 )->GetInteger();
				}
			}
			else
			{
				// Initialise ASoundEx
				ASound* pSoundEx = &pSoundBank->m_pSoundsEx[ iSoundIndex++ ];

				pSoundEx->m_iId = iSoundId;
				m_SoundIdToSound.Insert( iSoundId, pSoundEx );

				// Get category
				TPString8 strCategory;
				if ( bNoErrors && pSoundProperties->GetOptionalPropertyValue( strCategory, "category" ) )
				{
					TPSTRING8_DECLARE( default );

					// Find category and store it's index
					auto pFoundCategory = m_CategoryIndices.Find(
					    ( strCategory.GetPooledString() || strCategory.GetString8().Length() == 0 ) ? TPS8( default ) : strCategory
					);

					TASSERT( m_CategoryIndices.IsValid( pFoundCategory ) );
					pSoundEx->m_uiCategoryIndex = pFoundCategory->GetSecond();

					// Get flags
					if ( pSoundProperties->GetOptionalPropertyValue( pSoundEx->m_iFlags, "flags" ) )
					{
						// Get mindist and priority
						pSoundProperties->GetOptionalPropertyValue( pSoundEx->m_fMinDist, "mindist" );
						pSoundProperties->GetOptionalPropertyValue( pSoundEx->m_ui8Priority, "priority" );

						// Get banks and waves
						PBPropertyValueArray* pBanks;
						PBPropertyValueArray* pWaves;

						// Load info about the waves
						if ( pSoundProperties->GetOptionalPropertyValue( pBanks, "banks" ) &&
						     pSoundProperties->GetOptionalPropertyValue( pWaves, "waves" ) )
						{
							pSoundEx->m_vecSamples.Reserve( pBanks->GetSize() );

							for ( TUINT i = 0; i < pBanks->GetSize(); i++ )
							{
								// Find the wavebank
								TPString8  strWaveBankName = pBanks->GetValue( i )->GetTPString8();
								AWaveBank* pWaveBank       = FindWaveBank( strWaveBankName );
								TVALIDPTR( pWaveBank );

								if ( a_bLoadImmediately && !strWaveBankName.IsEmpty() && pWaveBank )
								{
									pWaveBank->Load( iLocalised != 0 ? AWaveBank::LOADFLAGS_LOCALISE : AWaveBank::LOADFLAGS_NONE, -1 );
								}

								if ( pWaveBank != TNULL )
								{
									ASound::Sample oWave;
									oWave.pWaveBank = pWaveBank;
									oWave.iId       = pWaves->GetValue( i )->GetInteger();

									pSoundEx->m_vecSamples.PushBack( oWave );

									if ( pSoundEx->m_iFlags & ASound::FLAGS_3D )
										pWaveBank->SetWaveFlag2( oWave.iId );
									else
										pWaveBank->SetWaveFlag1( oWave.iId );
								}
								else
								{
									bNoErrors = TFALSE;
								}
							}

							// Read volumes of the waves
							PBPropertyValueArray* pVolumes;
							if ( bNoErrors && pSoundProperties->GetOptionalPropertyValue( pVolumes, "volumes" ) )
							{
								for ( TUINT i = 0; i < pVolumes->GetSize(); i++ )
								{
									TFLOAT fVolume                      = pVolumes->GetValue( i )->GetFloat();
									pSoundEx->m_vecSamples[ i ].fVolume = fVolume;
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read randomization settings for the volumes
							PBPropertyValueArray* pVolrand;
							if ( pSoundProperties->GetOptionalPropertyValue( pVolrand, "volrand" ) )
							{
								for ( TUINT i = 0; i < pVolrand->GetSize(); i += 2 )
								{
									TFLOAT fVolMin = pVolrand->GetValue( i + 0 )->GetFloat();
									TFLOAT fVolMax = pVolrand->GetValue( i + 1 )->GetFloat();

									pSoundEx->m_vecSamples[ i / 2 ].fMinVolumeMultiplier = fVolMin;
									pSoundEx->m_vecSamples[ i / 2 ].fMaxVolumeMultiplier = fVolMax;
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read pitches of the waves
							PBPropertyValueArray* pPitches;
							if ( pSoundProperties->GetOptionalPropertyValue( pPitches, "pitches" ) )
							{
								for ( TUINT i = 0; i < pPitches->GetSize(); i++ )
								{
									TFLOAT fPitch                      = pPitches->GetValue( i )->GetFloat();
									pSoundEx->m_vecSamples[ i ].fPitch = fPitch;
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read randomization settings for the pitches
							PBPropertyValueArray* pPitchrand;
							if ( pSoundProperties->GetOptionalPropertyValue( pPitchrand, "pitchrand" ) )
							{
								for ( TUINT i = 0; i < pPitchrand->GetSize(); i += 2 )
								{
									TFLOAT fPitchMin = pPitchrand->GetValue( i + 0 )->GetFloat();
									TFLOAT fPitchMax = pPitchrand->GetValue( i + 1 )->GetFloat();

									pSoundEx->m_vecSamples[ i / 2 ].fMinPitch = fPitchMin;
									pSoundEx->m_vecSamples[ i / 2 ].fMaxPitch = fPitchMax;
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read start offsets of the waves
							PBPropertyValueArray* pStarts;
							if ( pSoundProperties->GetOptionalPropertyValue( pStarts, "starts" ) )
							{
								for ( TUINT i = 0; i < pStarts->GetSize(); i++ )
								{
									TFLOAT fStart                      = pStarts->GetValue( i )->GetFloat();
									pSoundEx->m_vecSamples[ i ].fStart = fStart;
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read var delays of the waves
							PBPropertyValueArray* pVarDelays;
							if ( pSoundProperties->GetOptionalPropertyValue( pVarDelays, "vardelays" ) )
							{
								for ( TUINT i = 0; i < pVarDelays->GetSize(); i++ )
								{
									TFLOAT fVarDelay                      = pVarDelays->GetValue( i )->GetFloat();
									pSoundEx->m_vecSamples[ i ].fVarDelay = fVarDelay;
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read tracks of the waves
							PBPropertyValueArray* pTracks;
							if ( pSoundProperties->GetOptionalPropertyValue( pTracks, "tracks" ) )
							{
								pSoundEx->m_vecTracks.Reserve( pTracks->GetSize() );

								for ( TUINT i = 0; i < pTracks->GetSize(); i++ )
								{
									pSoundEx->m_vecTracks.EmplaceBack( pTracks->GetValue( i )->GetInteger() );
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read weights of the waves
							PBPropertyValueArray* pWeights;
							if ( pSoundProperties->GetOptionalPropertyValue( pWeights, "weights" ) )
							{
								for ( TUINT i = 0; i < pWeights->GetSize(); i++ )
								{
									TFLOAT fWeight                      = pVarDelays->GetValue( i )->GetFloat();
									pSoundEx->m_vecSamples[ i ].iWeight = TMath::Round( fWeight );
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read trackloop settings of the waves
							PBPropertyValueArray* pTrackloop;
							if ( pSoundProperties->GetOptionalPropertyValue( pTrackloop, "trackloop" ) )
							{
								// Each bit represents a track and if it's set to 1, loop is enabled
								pSoundEx->m_TrackLoop.Create( pTrackloop->GetSize(), 0 );

								for ( TUINT i = 0; i < pTrackloop->GetSize(); i++ )
								{
									if ( pTrackloop->GetValue( i )->GetRaw() != 0 )
									{
										pSoundEx->m_TrackLoop.SetBit( i, 1 );
									}
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}

							// Read loopstarts of the waves
							PBPropertyValueArray* pLoopStarts;
							if ( pSoundProperties->GetOptionalPropertyValue( pLoopStarts, "loopstarts" ) )
							{
								for ( TUINT i = 0; i < pLoopStarts->GetSize(); i++ )
								{
									TINT iLoopStart = pLoopStarts->GetValue( i )->GetInteger();
									pSoundEx->m_vecLoopStarts.PushBack( iLoopStart );
								}
							}
							else
							{
								TASSERT( !"Error while parsing sound settings" );
								bNoErrors = TFALSE;
								continue;
							}
						}
					}
				}
			}
		}

		if ( bNoErrors )
		{
			if ( a_bLoadImmediately )
				pSoundBank->Load();
		}
		else
		{
			pSoundBank->Remove();
			delete pSoundBank;
		}
	}

	TUINT uiCurrentAlloced;
	TUINT uiMaxAlloced;
	FSOUND_GetMemoryStats( &uiCurrentAlloced, &uiMaxAlloced );
	TINFO( "LoadSoundBank result: name=%s; success=%d; currentalloced=%u; maxalloced=%u\n", a_szName, bNoErrors, uiCurrentAlloced, uiMaxAlloced );

	return bNoErrors;
}

TUINT ASoundManager::GetCueIndex( Cue* a_pCue )
{
	return ( TREINTERPRETCAST( TUINTPTR, a_pCue ) - TREINTERPRETCAST( TUINTPTR, ASoundManager::ms_pSingleton ) - TOFFSETOF( ASoundManager, m_aCues ) ) / sizeof( Cue );
}

// $Barnyard: FUNCTION 005d6300
void ASoundManager::GetMemoryStats( MemoryStats* a_pStats )
{
	TUtil::MemClear( a_pStats, sizeof( MemoryStats ) );
	FSOUND_GetMemoryStats( &a_pStats->uiCurrentAlloced, &a_pStats->uiMaxAlloced );
	a_pStats->uiUnknown = 0;
}

// $Barnyard: FUNCTION 005d6230
TBOOL ASoundManager::Initialise()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// Dev switches: "-background" (and "-mute") runs make no sound. Everything else keeps working, FMOD just has no output device.
	// "-devaudio" keeps the real output
	const TBOOL bMuted = ( T2String8::FindString( GetCommandLineA(), "-background" ) || T2String8::FindString( GetCommandLineA(), "-mute" ) ) && !T2String8::FindString( GetCommandLineA(), "-devaudio" );
	if ( bMuted ) FSOUND_SetOutput( FSOUND_OUTPUT_NOSOUND );
#endif

	FSOUND_Init( 44100, m_iMinHWChannels + m_iNumChannels, 0 );

	TINT num2DC, num3DC, numDC;
	FSOUND_GetNumHWChannels( &num2DC, &num3DC, &numDC );
	FSOUND_GetMaxChannels();
	FSOUND_Close();

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( bMuted ) FSOUND_SetOutput( FSOUND_OUTPUT_NOSOUND );
#endif

	FSOUND_SetMinHardwareChannels( m_bUseMinHardwareChannels ? m_iMinHWChannels : 0 );
	TBOOL bRes = FSOUND_Init( 44100, m_iNumChannels, 0 );
	FSOUND_GetNumHWChannels( &num2DC, &num3DC, &numDC );
	FSOUND_GetMaxChannels();
	FSOUND_Stream_SetBufferSize( 2000 );
	FSOUND_SetBufferSize( 100 );
	FSOUND_3D_SetDistanceFactor( 1.0f );
	FSOUND_3D_SetRolloffFactor( 1.0f );
	FSOUND_3D_SetDopplerFactor( 1.0f );

	return bRes;
}

// $Barnyard: FUNCTION 005d7540
void ASoundManager::PauseAllCues( TBOOL a_bPause )
{
	if ( a_bPause )
	{
		// Don't count number of pauses
		if ( ms_iNumPauses != 0 ) return;

		// Pause cues
		for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
		{
			Cue* pCue = &m_aCues[ i ];

			if ( !pCue->pSound || !m_aCategories[ pCue->pSound->m_uiCategoryIndex ].bPausable )
				continue;

			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( !( channel->iFlags & ASound::FLAGS_PAUSED ) )
				{
					FSOUND_SetPaused( channel->iFMODChannelHandle, TTRUE );
				}
			}
		}

		ms_iNumPauses += 1;
	}
	else if ( ms_iNumPauses > 0 && --ms_iNumPauses == 0 )
	{
		// Unpause cues
		for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
		{
			Cue* pCue = &m_aCues[ i ];

			if ( !pCue->pSound ) continue;

			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( !( channel->iFlags & ASound::FLAGS_PAUSED ) )
				{
					FSOUND_SetPaused( channel->iFMODChannelHandle, TFALSE );
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 005d7bb0
void ASoundManager::PauseCue( TINT a_iCueIndex, TBOOL a_bPause )
{
	if ( a_iCueIndex == -1 )
		return;

	TASSERT( a_iCueIndex >= 0 && a_iCueIndex < ASOUNDMANAGER_MAX_NUM_CUE );

	Cue* pCue = &m_aCues[ a_iCueIndex ];

	if ( !pCue->IsUsed() )
		return;

	if ( a_bPause )
	{
		if ( pCue->fStartTime2 <= 0.0f )
		{
			pCue->fStartTime2 = m_fCurrentTime;

			if ( pCue->oEventList.IsLinked() )
			{
				pCue->oEventList.Remove();
			}

			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				channel->iFlags |= ASound::FLAGS_PAUSED;

				if ( channel->iFMODChannelHandle != -1 )
				{
					FSOUND_SetPaused( channel->iFMODChannelHandle, TTRUE );
				}
			}
		}
	}
	else if ( pCue->fStartTime2 > 0.0f )
	{
		pCue->fStartTime += m_fCurrentTime - pCue->fStartTime2;
		pCue->fStartTime2 = -1.0f;

		if ( !pCue->oEventList.IsLinked() && !pCue->oEventList->IsEmpty() )
		{
			m_QueuedSortedEventLists.ReInsert( pCue->oEventList );
		}

		T2_FOREACH( pCue->oChannelRefs, channel )
		{
			channel->iFlags &= ~ASound::FLAGS_PAUSED;

			if ( channel->iFMODChannelHandle != -1 )
			{
				FSOUND_SetPaused( channel->iFMODChannelHandle, TFALSE );
			}
		}
	}
}

// $Barnyard: FUNCTION 005dbd40
TBOOL ASoundManager::LoadWaveBanksInfo( const TCHAR* a_szFileName )
{
	TBOOL bOpened = AAssetLoader::Load(
	    "Data/Assets/lib_wavebank.trb",
	    AAssetType_WaveBank,
	    TTRUE
	);

	if ( !bOpened ) return TFALSE;

	const PBProperties* pProperties =
	    AAssetLoader::CastSymbol<const PBProperties>( a_szFileName, PBProperties::TRB_SECTION_NAME, AAssetType_WaveBank );
	TVALIDPTR( pProperties );

	auto pWaveBanksVal = pProperties->GetProperty( "Wavebanks" );
	TVALIDPTR( pWaveBanksVal );

	auto pWaveBanks = pWaveBanksVal->GetArray();

	for ( TUINT i = 0; i < pWaveBanks->GetSize(); i++ )
	{
		TPString8 strWaveBankName = pWaveBanks->GetValue( i )->GetTPString8();

		if ( FindWaveBank( strWaveBankName ) == TNULL )
		{
			LoadWaveBankFromAsset( strWaveBankName.GetString(), 0 );
		}
	}

	auto pCategoriesVal = pProperties->GetProperty( "Categories" );
	TVALIDPTR( pCategoriesVal );

	auto pCategories = pCategoriesVal->GetArray();

	for ( TUINT i = 0; i < pCategories->GetSize(); i++ )
	{
		TPString8 strCategoryName = pCategories->GetValue( i )->GetTPString8();

		m_CategoryIndices.Insert( strCategoryName, m_CategoryIndices.Size() );
	}

	AAssetLoader::Close( AAssetType_WaveBank );

	return TTRUE;
}

// $Barnyard: FUNCTION 005d6830
TBOOL ASoundManager::LoadWaveBankSamples( const Toshi::TPString8& a_rcName, AWaveBank::LOADFLAGS a_eLoadFlags, TINT a_iBufferSize )
{
	if ( a_rcName.GetPooledString() && !a_rcName.GetPooledString()->GetString8().IsEmpty() )
	{
		AWaveBank* pWaveBank = FindWaveBank( a_rcName );

		if ( pWaveBank )
		{
			return pWaveBank->Load( a_eLoadFlags, a_iBufferSize ) == AWaveBank::LOADRESULT_OK;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005d6880
void ASoundManager::UnloadWaveBankSamples( const Toshi::TPString8& a_rcName )
{
	if ( a_rcName.GetPooledString() && !a_rcName.GetPooledString()->GetString8().IsEmpty() )
	{
		AWaveBank* pWaveBank = FindWaveBank( a_rcName );

		if ( pWaveBank )
			pWaveBank->Unload();
	}
}

ASoundBank* ASoundManager::FindSoundBank( const Toshi::TPString8& a_rcName )
{
	T2_FOREACH( m_SoundBanks, it )
	{
		if ( it->m_strName == a_rcName )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005da750
TBOOL ASoundManager::LoadSoundBank( const Toshi::TPString8& a_rcName, TBOOL a_bSoundEx, TBOOL a_bLoadImmediately )
{
	if ( a_rcName.GetPooledString() && !a_rcName.GetPooledString()->GetString8().IsEmpty() )
	{
		if ( FindSoundBank( a_rcName ) == TNULL )
		{
			return LoadSoundBankImpl( a_rcName, a_bSoundEx, a_bLoadImmediately );
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005d79c0
void ASoundManager::LoadSoundBankSamples( const Toshi::TPString8& a_rcName )
{
	if ( ASoundBank* pSoundBank = FindSoundBank( a_rcName ) )
	{
		pSoundBank->Load();
	}
}

// $Barnyard: FUNCTION 005db4a0
void ASoundManager::UnloadSoundBank( const Toshi::TPString8& a_rcName, TBOOL a_bSimpleSound )
{
	for ( auto it = m_SoundBanks.Begin(); it != m_SoundBanks.End(); )
	{
		ASoundBank* pSoundBank = it;
		it++;

		if ( pSoundBank->m_strName != a_rcName )
			continue;

		if ( a_bSimpleSound )
		{
			for ( TINT i = 0; i < pSoundBank->m_iNumSounds; i++ )
			{
				ASoundLegacy* pSound = &pSoundBank->m_pSounds[ i ];

				m_SoundIdToSoundLegacy.FindAndRemove( pSound->m_iId );
				pSound->m_pWaveBank->Unload();
			}

			// NOTE: retail frees the array with TFree, ASoundLegacy has no destructor
			delete[] pSoundBank->m_pSounds;
			pSoundBank->m_pSounds = TNULL;
		}
		else
		{
			for ( TINT i = 0; i < pSoundBank->m_iNumSounds; i++ )
			{
				ASound* pSound = &pSoundBank->m_pSoundsEx[ i ];

				// Stop every cue that is playing this sound
				TBOOL bStoppedCue = TFALSE;

				for ( TINT k = 0; k < ASOUNDMANAGER_MAX_NUM_CUE; k++ )
				{
					if ( m_aCues[ k ].pSound == pSound )
					{
						bStoppedCue = TTRUE;

						TINT iCueIndex = k;
						StopCueAsync( iCueIndex, 0.0f );
					}
				}

				// Process the stop events right away
				if ( bStoppedCue )
					OnUpdate( g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() );

				m_SoundIdToSound.FindAndRemove( pSound->m_iId );

				for ( TINT k = 0; k < pSound->m_vecSamples.Size(); k++ )
				{
					pSound->m_vecSamples[ k ].pWaveBank->Unload();
				}
			}

			delete[] pSoundBank->m_pSoundsEx;
			pSoundBank->m_pSoundsEx = TNULL;
		}

		pSoundBank->Remove();
		delete pSoundBank;
	}
}

// $Barnyard: FUNCTION 005db1a0
void ASoundManager::LoadSoundBanksFromProperty( const PBPropertyValue* a_pBankList )
{
	if ( !a_pBankList )
		return;

	const PBPropertyValueArray* pBanks   = a_pBankList->GetArray();
	APlayerSimProfile*          pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	for ( TUINT i = 0; i < pBanks->GetSize(); i++ )
	{
		TPString8 strBankName = pBanks->GetValue( i )->GetTPString8();

		// Only load voice of the player's gender
		if ( strBankName == TPS8( am_m ) && pProfile->GetGender() == APlayerProfile::GENDER_FEMALE )
			continue;

		if ( strBankName == TPS8( am_f ) && pProfile->GetGender() == APlayerProfile::GENDER_MALE )
			continue;

		LoadSoundBank( pBanks->GetValue( i )->GetTPString8(), TFALSE, TTRUE );
	}
}

// $Barnyard: FUNCTION 005dbca0
void ASoundManager::UnloadSoundBanksFromProperty( const PBPropertyValue* a_pBankList )
{
	if ( !a_pBankList )
		return;

	const PBPropertyValueArray* pBanks = a_pBankList->GetArray();

	for ( TUINT i = 0; i < pBanks->GetSize(); i++ )
	{
		UnloadSoundBank( pBanks->GetValue( i )->GetTPString8(), TFALSE );
	}
}

// $Barnyard: FUNCTION 005d8690
void ASoundManager::CreatePlaySoundEvent( Cue* a_pCue, TINT a_iTrackIndex, TINT a_iFirstWaveIndex, TINT a_iLastWaveIndex, TINT a_iFlags, TFLOAT a_fDelay1, TFLOAT a_fDelay2 )
{
	TVALIDPTR( a_pCue );

	ASound* pSound = a_pCue->pSound;
	TVALIDPTR( pSound );

	TINT iFlags = a_iFlags;

	TINT iLoopStart = pSound->m_vecLoopStarts[ a_iTrackIndex ];
	if ( iLoopStart > 0 || iLoopStart == -1 )
		iFlags &= ~ASound::FLAGS_LOOP;

	TRandom* pRandom = &ARandom::GetSingleton()->m_oRandom;

	if ( !HASANYFLAG( iFlags, ASound::FLAGS_UNK3 ) )
	{
		// Play multiple sounds
		TASSERT( a_iFirstWaveIndex <= a_iLastWaveIndex );

		if ( pSound->m_vecTracks[ a_iTrackIndex ] < 2 )
		{
			TINT iNumLeftWaves = a_iLastWaveIndex - a_iFirstWaveIndex;

			for ( TINT i = a_iFirstWaveIndex; iNumLeftWaves > 0; iNumLeftWaves-- )
			{
				ASound::Sample* pSample = &pSound->m_vecSamples[ i ];

				TFLOAT fVolumeMultiplier = pRandom->GetFloatMinMax( pSample->fMinVolumeMultiplier, pSample->fMaxVolumeMultiplier );
				TFLOAT fVolume           = fVolumeMultiplier * pSample->fVolume;
				TMath::Clip( fVolume, 0.0f, 1.0f );

				EventParameters oParams;
				oParams[ 0 ] = fVolume;
				oParams[ 1 ] = pRandom->GetFloatMinMax( pSample->fMinPitch, pSample->fMaxPitch );
				oParams[ 2 ] = 0.0f;

				TFLOAT fStartDelay = pSample->fStart + pRandom->GetFloat() * pSample->fVarDelay;
				if ( a_fDelay1 != -1.0f ) fStartDelay += a_fDelay1;
				if ( a_fDelay2 != -1.0f ) fStartDelay += a_fDelay2;

				SoundEvent* pEvent = CreateSoundEvent(
				    ( iFlags & ASound::FLAGS_STREAM ) ? SOUNDEVENT_PlayStream : SOUNDEVENT_PlayAudio,
				    fStartDelay,
				    a_pCue,
				    pSample,
				    oParams,
				    TNULL,
				    iFlags,
				    a_iTrackIndex
				);

				TVALIDPTR( pEvent );
			}
		}
	}
	else
	{
		// Play single sound

		TINT iSampleIndex = ( a_iFlags < 0 && pSound->m_vecTracks[ a_iTrackIndex ] > 1 ) ?
		    pSound->GetRandomSampleWeighted( a_iTrackIndex, a_iFirstWaveIndex, a_iLastWaveIndex ) :
		    pSound->GetRandomSample( a_iFirstWaveIndex, a_iLastWaveIndex );

		ASound::Sample* pSample = &pSound->m_vecSamples[ iSampleIndex ];

		TFLOAT fVolumeMultiplier = pRandom->GetFloatMinMax( pSample->fMinVolumeMultiplier, pSample->fMaxVolumeMultiplier );
		TFLOAT fVolume           = fVolumeMultiplier * pSample->fVolume;
		TMath::Clip( fVolume, 0.0f, 1.0f );

		EventParameters oParams;
		oParams[ 0 ] = fVolume;
		oParams[ 1 ] = pRandom->GetFloatMinMax( pSample->fMinPitch, pSample->fMaxPitch );
		oParams[ 2 ] = 0.0f;

		TFLOAT fStartDelay = pSample->fStart + pRandom->GetFloatMinMax( 0.0f, pSample->fVarDelay );
		if ( a_fDelay1 != -1.0f ) fStartDelay += a_fDelay1;
		if ( a_fDelay2 != -1.0f ) fStartDelay += a_fDelay2;

		SoundEvent* pEvent = CreateSoundEvent(
		    ( iFlags & ASound::FLAGS_STREAM ) ? SOUNDEVENT_PlayStream : SOUNDEVENT_PlayAudio,
		    fStartDelay,
		    a_pCue,
		    pSample,
		    oParams,
		    TNULL,
		    iFlags,
		    a_iTrackIndex
		);

		TVALIDPTR( pEvent );
	}
}

// $Barnyard: FUNCTION 005d8230
ASoundManager::SoundEvent* ASoundManager::CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pSample, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex )
{
	TVALIDPTR( a_pCue );
	TASSERT( m_SoundEventPool.CanAllocate() );

	if ( m_SoundEventPool.CanAllocate() )
	{
		SoundEvent* pEvent  = m_SoundEventPool.NewObject();
		pEvent->pSample     = a_pSample;
		pEvent->eEventType  = a_eEventType;
		pEvent->pChannel    = a_pChannel;
		pEvent->iFlags      = a_iFlags;
		pEvent->pCue        = a_pCue;
		pEvent->fStartTime  = a_fDelay + m_fCurrentTime;
		pEvent->iTrackIndex = a_iTrackIndex;
		TVALIDPTR( pEvent );

		AddEventToCue( a_pCue, pEvent );
		return pEvent;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005d8290
ASoundManager::SoundEvent* ASoundManager::CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pSample, TFLOAT a_fCustomParam1, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex )
{
	TVALIDPTR( a_pCue );
	TASSERT( m_SoundEventPool.CanAllocate() );

	if ( m_SoundEventPool.CanAllocate() )
	{
		SoundEvent* pEvent       = m_SoundEventPool.NewObject();
		pEvent->pSample          = a_pSample;
		pEvent->eEventType       = a_eEventType;
		pEvent->pChannel         = a_pChannel;
		pEvent->iFlags           = a_iFlags;
		pEvent->pCue             = a_pCue;
		pEvent->oParameters[ 0 ] = a_fCustomParam1;
		pEvent->iTrackIndex      = a_iTrackIndex;
		pEvent->fStartTime       = a_fDelay + m_fCurrentTime;
		TVALIDPTR( pEvent );

		AddEventToCue( a_pCue, pEvent );
		return pEvent;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005d8300
ASoundManager::SoundEvent* ASoundManager::CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pSample, const EventParameters& a_rcCustomParams, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex )
{
	TVALIDPTR( a_pCue );
	TASSERT( m_SoundEventPool.CanAllocate() );

	if ( m_SoundEventPool.CanAllocate() )
	{
		SoundEvent* pEvent  = m_SoundEventPool.NewObject();
		pEvent->eEventType  = a_eEventType;
		pEvent->pSample     = a_pSample;
		pEvent->pChannel    = a_pChannel;
		pEvent->pCue        = a_pCue;
		pEvent->iFlags      = a_iFlags;
		pEvent->oParameters = a_rcCustomParams;
		pEvent->fStartTime  = a_fDelay + m_fCurrentTime;
		pEvent->iTrackIndex = a_iTrackIndex;
		TVALIDPTR( pEvent );

		AddEventToCue( a_pCue, pEvent );
		return pEvent;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005d7cf0
void ASoundManager::AddEventToCue( Cue* a_pCue, SoundEvent* a_pSoundEvent )
{
	TVALIDPTR( a_pCue );
	TVALIDPTR( a_pSoundEvent );

	SoundEvent* pFirstEvent = ( !a_pCue->oEventList->IsEmpty() ) ? &*a_pCue->oEventList->Begin() : TNULL;
	a_pCue->oEventList->Push( a_pSoundEvent );

	// If first event in the list changed, reinsert the list in m_QueuedEventLists
	// to make sure the ordering is right. It's important because the sorting method
	// is taking the first event into account when sorting the lists
	if ( pFirstEvent != a_pCue->oEventList->Begin() )
	{
		m_QueuedSortedEventLists.ReInsert( a_pCue->oEventList );
	}
}

// $Barnyard: FUNCTION 005d8380
TBOOL ASoundManager::UpdateStreamActivity( StreamRef* a_pStream )
{
	S2*           pSlot  = TSTATICCAST( S2, a_pStream->pLegacySlot );
	ASoundLegacy* pSound = pSlot->pSound;

	if ( !pSound )
		return TTRUE;

	ChannelRef*     pChannel      = a_pStream->pChannelRef;
	ASound::Sample* pSample       = pChannel->pLegacySample;
	FSOUND_STREAM*  pStreamHandle = (FSOUND_STREAM*)pSample->pWaveBank->CreateWaveSample( pSample->iId, 0 );

	if ( !( pSound->m_iFlags & ASound::FLAGS_STREAM_OPENING ) )
	{
		FSOUND_Stream_GetNumSubStreams( pStreamHandle );
		FSOUND_Stream_SetSubStream( pStreamHandle, pSample->pWaveBank->GetWaveId( pSample->iId ) );
		pSound->m_iFlags |= ASound::FLAGS_STREAM_OPENING;
	}

	TINT iOpenState = FSOUND_Stream_GetOpenState( pStreamHandle );

	if ( iOpenState != OPENSTATE_INVALID )
	{
		if ( iOpenState == OPENSTATE_OPENING )
			return TFALSE;

		if ( iOpenState != OPENSTATE_FAILED )
		{
			if ( iOpenState != OPENSTATE_READY )
				return TTRUE;

			// The stream is open, play it
			pSound->m_iFlags &= ~ASound::FLAGS_STREAM_OPENING;

			pChannel->iFMODChannelHandle = FSOUND_Stream_PlayEx( FSOUND_FREE, pStreamHandle, TNULL, TTRUE );

			if ( pChannel->iFMODChannelHandle == -1 )
				return TTRUE;

			pChannel->iFlags |= ASound::FLAGS_STARTED;
			pSound->m_iFlags |= ASound::FLAGS_STARTED;
			TINT iFlags = pChannel->iFlags;

			FSOUND_SetVolume( pChannel->iFMODChannelHandle, ( m_bMuted == TTRUE ) ? 0 : TINT( pChannel->fVolume * 255.0f ) );
			FSOUND_SetFrequency( pChannel->iFMODChannelHandle, pChannel->iFrequency );
			FSOUND_SetPriority( pChannel->iFMODChannelHandle, 255 );

			TUINT uiMode = FSOUND_Stream_GetMode( pStreamHandle );
			uiMode       = ( iFlags & ASound::FLAGS_LOOP ) ? ( uiMode & 0xfffffff8 | 2 ) : ( uiMode & 0xfffffff8 | 1 );
			uiMode       = ( iFlags & ASound::FLAGS_3D ) ? ( uiMode & 0xfff7cfff | 0x1000 ) : ( uiMode & 0xfff7cfff | 0x80000 );
			FSOUND_Stream_SetMode( pStreamHandle, uiMode );

			if ( !( iFlags & ASound::FLAGS_3D ) )
				FSOUND_SetPan( pChannel->iFMODChannelHandle, 128 );

			FSOUND_SetPaused( pChannel->iFMODChannelHandle, TFALSE );
			return TTRUE;
		}
	}

	// Failed to open the stream
	pSound->m_iFlags &= ~ASound::FLAGS_STREAM_OPENING;
	pSlot->Reset();
	return TTRUE;
}

// $Barnyard: FUNCTION 005d7f00
void ASoundManager::S2::Reset()
{
	fStartTime    = -1.0f;
	fVolume       = -1.0f;
	pSound        = TNULL;
	bStreamQueued = TFALSE;

	if ( pChannelRef )
	{
		FSOUND_StopSound( pChannelRef->iFMODChannelHandle );
		pChannelRef->pLegacySample->pWaveBank->ResetWave( pChannelRef->pLegacySample->iId );
		ASoundManager::GetSingleton()->m_ChannelRefPool.DeleteObject( pChannelRef );
		pChannelRef = TNULL;
	}
}

// $Barnyard: FUNCTION 005da190
TBOOL ASoundManager::UpdateStreamCue( StreamRef* a_pStream )
{
	TVALIDPTR( a_pStream );

	Cue*    pCue   = a_pStream->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound )
	{
		ChannelRef*    pChannel      = a_pStream->pChannelRef;
		AWaveBank*     pWaveBank     = pChannel->pSample->pWaveBank;
		FSOUND_STREAM* pStreamHandle = (FSOUND_STREAM*)pWaveBank->CreateWaveSample( pChannel->pSample->iId, 0 );

		if ( !( pSound->m_iFlags & ASound::FLAGS_STREAM_OPENING ) )
		{
			TINT iOpenState = FSOUND_Stream_GetOpenState( pStreamHandle );

			if ( iOpenState == OPENSTATE_OPENING ) return TFALSE;
			FSOUND_Stream_GetNumSubStreams( pStreamHandle );
			FSOUND_Stream_SetSubStream( pStreamHandle, pWaveBank->GetWaveId( pChannel->pSample->iId ) );

			pSound->m_iFlags |= ASound::FLAGS_STREAM_OPENING;
		}

		TINT iOpenState = FSOUND_Stream_GetOpenState( pStreamHandle );

		if ( iOpenState != OPENSTATE_INVALID )
		{
			if ( iOpenState == OPENSTATE_OPENING )
				return TFALSE;

			if ( iOpenState != OPENSTATE_FAILED )
			{
				if ( iOpenState != OPENSTATE_READY )
					return TTRUE;

				pSound->m_iFlags &= ~ASound::FLAGS_STREAM_OPENING;

				TINT iChannelHandle          = FSOUND_Stream_PlayEx( FSOUND_FREE, pStreamHandle, TNULL, TTRUE );
				pChannel->iFMODChannelHandle = iChannelHandle;
				TASSERT( iChannelHandle != -1 );

				if ( iChannelHandle == -1 )
					return TTRUE;

				pChannel->iFlags = pChannel->iFlags | ASound::FLAGS_STARTED;
				pSound->m_iFlags |= ASound::FLAGS_STARTED;

				TINT iVolume = ( m_bMuted ) ? 0 : TINT( pChannel->fVolume * 255.0f );

				FSOUND_SetVolume( iChannelHandle, iVolume );
				FSOUND_SetFrequency( iChannelHandle, pChannel->iFrequency );
				FSOUND_SetPriority( iChannelHandle, TINT( pSound->m_ui8Priority ) );

				TINT iStreamMode = FSOUND_Stream_GetMode( pStreamHandle );

				if ( pChannel->iFlags & ASound::FLAGS_LOOP )
				{
					iStreamMode = iStreamMode & 0xfffffff8 | 2;
					FSOUND_SetReserved( iChannelHandle, TTRUE );
				}
				else
				{
					iStreamMode = iStreamMode & 0xfffffff8 | 1;
				}

				if ( pChannel->iFlags & ASound::FLAGS_3D )
					iStreamMode = iStreamMode & 0xfff7dfff | 0x1000;
				else
					iStreamMode = iStreamMode & 0xffffcfff | 0x80000;

				FSOUND_Stream_SetMode( pStreamHandle, iStreamMode );

				if ( pChannel->iFlags & ASound::FLAGS_3D )
				{
					TFLOAT aPos[ 3 ];
					aPos[ 0 ] = ( pCue->vecPosition ).x;
					aPos[ 1 ] = ( pCue->vecPosition ).y;
					aPos[ 2 ] = -( pCue->vecPosition ).z;

					FSOUND_3D_SetMinMaxDistance( iChannelHandle, pSound->m_fMinDist, 10000.0f );
					FSOUND_3D_SetAttributes( iChannelHandle, aPos, TNULL );
				}
				else
				{
					FSOUND_SetPan( iChannelHandle, 128 );
				}

				if ( !HASANYFLAG( pChannel->iFlags, ASound::FLAGS_PAUSED ) )
				{
					FSOUND_SetPaused( iChannelHandle, TFALSE );
				}
				else if ( pCue->oEventList.IsLinked() )
				{
					pCue->oEventList.Remove();
				}

				T2_FOREACH( pCue->aChannelRefsLegacy[ 0 ], it )
				{
					it->fnCallback( 0, GetCueIndex( pCue ), it->pUserData );
				}

				return TTRUE;
			}
		}

		pSound->m_iFlags = pSound->m_iFlags & ~ASound::FLAGS_STREAM_OPENING;
		pCue->Reset();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005da3f0
void ASoundManager::EventHandler_PlaySound( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );
	TASSERT( TTRUE == m_ChannelRefPool.CanAllocate() );

	Cue*    pCue   = a_pEvent->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound && m_ChannelRefPool.CanAllocate() )
	{
		// Allocate new ChannelRef object from the pool
		ChannelRef* pChannelRef = m_ChannelRefPool.NewObject();

		// Push the channelref to the cue
		pCue->iNumChannelRefs += 1;
		pCue->oChannelRefs.PushBack( pChannelRef );

		TFLOAT fCategoryVolume = m_aCategories[ pCue->pSound->m_uiCategoryIndex ].fVolumeMultiplier;

		pChannelRef->pSample = a_pEvent->pSample;
		pChannelRef->fVolume = fCategoryVolume * pCue->fVolume * a_pEvent->oParameters[ 0 ];
		TMath::Clip( pChannelRef->fVolume, 0.0f, 1.0f );

		ASound::Sample* pWave   = pChannelRef->pSample;
		pChannelRef->iFlags     = a_pEvent->iFlags;
		pChannelRef->iFrequency = TINT( pWave->iFrequency * a_pEvent->oParameters[ 1 ] );

		FSOUND_SAMPLE* pSample = (FSOUND_SAMPLE*)pWave->pWaveBank->GetWaveSampleHandle( pWave->iId );

		TINT iSampleMode = FSOUND_Sample_GetMode( pSample );

		if ( pChannelRef->iFlags & ASound::FLAGS_LOOP )
			iSampleMode = iSampleMode & 0xfffffffa | 2;
		else
			iSampleMode = iSampleMode & 0xfffffff9 | 1;

		FSOUND_Sample_SetMode( pSample, iSampleMode );

		TINT iChannelHandle             = FSOUND_PlaySoundEx( FSOUND_FREE, pSample, TNULL, TTRUE );
		pChannelRef->iFMODChannelHandle = iChannelHandle;
		TASSERT( iChannelHandle != -1 );

		if ( iChannelHandle != -1 )
		{
			// No errors
			TINT iVolume = ( m_bMuted ) ? 0 : TINT( pChannelRef->fVolume * 255.0f );

			FSOUND_SetVolume( iChannelHandle, iVolume );
			FSOUND_SetFrequency( iChannelHandle, pChannelRef->iFrequency );
			FSOUND_SetPriority( iChannelHandle, TINT( pSound->m_ui8Priority ) );

			if ( pChannelRef->iFlags & ASound::FLAGS_3D )
			{
				TFLOAT aPos[ 3 ];
				aPos[ 0 ] = ( pCue->vecPosition ).x;
				aPos[ 1 ] = ( pCue->vecPosition ).y;
				aPos[ 2 ] = -( pCue->vecPosition ).z;

				FSOUND_3D_SetMinMaxDistance( iChannelHandle, pSound->m_fMinDist, 10000.0f );
				FSOUND_3D_SetAttributes( iChannelHandle, aPos, TNULL );
			}
			else
			{
				FSOUND_SetPan( iChannelHandle, 128 );
			}

			if ( !HASANYFLAG( pChannelRef->iFlags, ASound::FLAGS_PAUSED ) )
			{
				FSOUND_SetPaused( iChannelHandle, TFALSE );
			}
			else if ( pCue->oEventList.IsLinked() )
			{
				pCue->oEventList.Remove();
			}

			pSound->m_iFlags |= ASound::FLAGS_STARTED;
			pChannelRef->iFlags |= ASound::FLAGS_STARTED;

			ASoundManager* pSoundManager = ASoundManager::GetSingleton();

			T2_FOREACH( pCue->aChannelRefsLegacy[ 0 ], it )
			{
				it->fnCallback( 0, GetCueIndex( pCue ), it->pUserData );
			}

			TINT iTrackIndex = a_pEvent->iTrackIndex;

			if ( iTrackIndex != -1 )
			{
				TINT iLoopStart = pCue->vecLoopStarts[ iTrackIndex ];

				if ( iLoopStart == -1 )
				{
					TINT iFirstWave = pSound->GetFirstWaveForTrack( iTrackIndex );

					// Remove channel refs which has finished playing
					for ( auto it = pCue->oChannelRefs.Begin(); it != pCue->oChannelRefs.End(); )
					{
						if ( !FSOUND_IsPlaying( it->iFMODChannelHandle ) )
						{
							ChannelRef* pChannel = it;
							it                   = pCue->oChannelRefs.Erase( it );
							m_ChannelRefPool.FreeObject( pChannel );

#ifndef BARNYARD_COMMUNITY_PATCH
							// NOTE: retail skips the node after the removed one since the loop moves
							// to the next node of the one Erase returns
							if ( it != pCue->oChannelRefs.End() )
								it = it.Next();
#endif
						}
						else
						{
							it = it.Next();
						}
					}

					CreatePlaySoundEvent(
					    pCue,
					    iTrackIndex,
					    iFirstWave,
					    pSound->m_vecTracks[ iTrackIndex ] + iFirstWave,
					    a_pEvent->iFlags,
					    0.0f,
					    pChannelRef->pSample->pWaveBank->GetWaveLength( pChannelRef->pSample->iId )
					);
				}

				if ( iLoopStart > 0 )
				{
					pCue->vecLoopStarts[ iTrackIndex ] -= 1;
					TINT iFirstWave = pSound->GetFirstWaveForTrack( iTrackIndex );

					CreatePlaySoundEvent(
					    pCue,
					    iTrackIndex,
					    iFirstWave,
					    pSound->m_vecTracks[ iTrackIndex ] + iFirstWave,
					    a_pEvent->iFlags,
					    0.0f,
					    pChannelRef->pSample->pWaveBank->GetWaveLength( pChannelRef->pSample->iId )
					);
				}
			}
		}
		else
		{
			// Error occured while playing
			TASSERT( !"Couldn't play sound sample" );
			pCue->Reset();
		}
	}
}

// $Barnyard: FUNCTION 005d7720
void ASoundManager::EventHandler_PlayStream( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );
	TASSERT( TTRUE == m_ChannelRefPool.CanAllocate() );

	Cue*    pCue   = a_pEvent->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound && m_ChannelRefPool.CanAllocate() )
	{
		// Allocate new ChannelRef object from the pool
		ChannelRef* pChannelRef = m_ChannelRefPool.NewObject();

		// Push the channelref to the cue
		pCue->iNumChannelRefs += 1;
		pCue->oChannelRefs.PushBack( pChannelRef );

		TFLOAT fCategoryVolume = m_aCategories[ pCue->pSound->m_uiCategoryIndex ].fVolumeMultiplier;

		pChannelRef->pSample = a_pEvent->pSample;
		pChannelRef->fVolume = fCategoryVolume * pCue->fVolume * a_pEvent->oParameters[ 0 ];
		TMath::Clip( pChannelRef->fVolume, 0.0f, 1.0f );

		ASound::Sample* pWave   = pChannelRef->pSample;
		pChannelRef->iFlags     = a_pEvent->iFlags & ( ~ASound::FLAGS_STARTED );
		pChannelRef->iFrequency = TINT( pWave->iFrequency * a_pEvent->oParameters[ 1 ] );

		TASSERT( TTRUE == m_StreamRefPool.CanAllocate() );

		if ( m_StreamRefPool.CanAllocate() )
		{
			StreamRef* pStreamRef   = m_StreamRefPool.NewObject();
			pStreamRef->pChannelRef = pChannelRef;
			pStreamRef->pCue        = pCue;

			pChannelRef->iFMODChannelHandle = -1;
			m_StreamRefs.PushBack( pStreamRef );
		}
	}
}

// $Barnyard: FUNCTION 005d7d50
void ASoundManager::EventHandler_StopAudio( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );

	Cue*    pCue   = a_pEvent->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound )
	{
		ChannelRef* pChannel  = a_pEvent->pChannel;
		TBOOL       bReserved = FSOUND_GetReserved( pChannel->iFMODChannelHandle );

		if ( bReserved )
			FSOUND_SetReserved( pChannel->iFMODChannelHandle, TFALSE );

		FSOUND_StopSound( pChannel->iFMODChannelHandle );

		if ( pSound->m_iFlags & ASound::FLAGS_STREAM )
		{
			FSOUND_STREAM* pStreamHandle = (FSOUND_STREAM*)pChannel->pSample->pWaveBank->GetWaveSampleHandle( pChannel->pSample->iId );
			FSOUND_Stream_Stop( pStreamHandle );
		}

		pChannel->pSample->pWaveBank->ResetWave( pChannel->pSample->iId );
		pCue->iNumChannelRefs -= 1;

		pChannel->Remove();
		m_ChannelRefPool.DeleteObject( pChannel );
	}
}

// $Barnyard: FUNCTION 005d6700
void ASoundManager::EventHandler_UpdateChannelVolume( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );

	Cue*    pCue   = a_pEvent->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound )
	{
		ChannelRef* pChannel = a_pEvent->pChannel;

		pChannel->fVolume = m_aCategories[ pSound->m_uiCategoryIndex ].fVolumeMultiplier * pChannel->pSample->fVolume * a_pEvent->oParameters[ 0 ];
		TMath::Clip( pChannel->fVolume, 0.0f, 1.0f );

		pCue->fVolume = a_pEvent->oParameters[ 0 ];

		FSOUND_SetVolume( pChannel->iFMODChannelHandle, ( !m_bMuted ) ? TINT( pChannel->fVolume * 255.0f ) : 0 );
	}
}

// $Barnyard: FUNCTION 005d61c0
void ASoundManager::EventHandler_UpdateChannelFrequency( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );

	Cue*    pCue   = a_pEvent->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound )
	{
		ChannelRef* pChannel = a_pEvent->pChannel;

		pChannel->iFrequency = TINT( a_pEvent->pSample->iFrequency * pCue->fFrequency * a_pEvent->oParameters[ 0 ] );
		FSOUND_SetFrequency( pChannel->iFMODChannelHandle, pChannel->iFrequency );
	}
}

// $Barnyard: FUNCTION 005d6200
void ASoundManager::EventHandler_UpdatePosition( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );

	Cue*    pCue   = a_pEvent->pCue;
	ASound* pSound = pCue->pSound;

	if ( pSound )
	{
		ChannelRef* pChannel = a_pEvent->pChannel;

		FSOUND_3D_SetAttributes( pChannel->iFMODChannelHandle, a_pEvent->oParameters.aParams, TNULL );
	}
}

// $Barnyard: FUNCTION 005d7dd0
ASoundManager::Cue::Cue()
{

	bUsed       = TFALSE;
	pSound      = TNULL;
	fStartTime  = 0.0f;
	fStartTime2 = 0.0f;
	vecPosition = TVector4::VEC_ZERO;
	fVolume     = 1.0f;
	fFrequency  = 1.0f;
	oEventList  = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) SoundEventList::Type();
}

// $Barnyard: FUNCTION 005d7ea0
ASoundManager::Cue::~Cue()
{
	if ( oEventList.GetNodeValue() )
		delete oEventList.GetNodeValue();
}

// $Barnyard: FUNCTION 005d8500
void ASoundManager::Cue::Reset()
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	T2_FOREACH( aChannelRefsLegacy[ 1 ], it )
	{
		it->fnCallback( 1, GetCueIndex( this ), it->pUserData );
	}

	bUsed       = TFALSE;
	fStartTime  = -1.0f;
	fStartTime2 = -1.0f;
	pSound      = TNULL;

	// Reset waves used by the channel refs
	T2_FOREACH( oChannelRefs, channelRef )
	{
		channelRef->pSample->pWaveBank->ResetWave( channelRef->pSample->iId );
	}

	// Delete allocated sound events
	while ( !oEventList->IsEmpty() )
	{
		SoundEvent* pSoundEvent = oEventList->PopFront();
		pSoundManager->m_SoundEventPool.FreeObject( pSoundEvent );
	}

	if ( oEventList.IsLinked() )
		oEventList.Remove();

	// Delete all channel refs of Type2
	for ( TINT i = 0; i < TARRAYSIZE( aChannelRefsLegacy ); i++ )
	{
		while ( !aChannelRefsLegacy[ i ].IsEmpty() )
		{
			ChannelRefLegacy* pChannelRef = aChannelRefsLegacy[ i ].PopFront();
			pSoundManager->m_ChannelRefLegacyPool.DeleteObject( pChannelRef );
		}
	}

	iNumChannelRefs = 0;

	while ( !oChannelRefs.IsEmpty() )
	{
		ChannelRef* pChannelRef = oChannelRefs.PopFront();
		pSoundManager->m_ChannelRefPool.DeleteObject( pChannelRef );
	}

	vecLoopStarts.Clear();

	if ( IsLinked() )
		Remove();
}

TBOOL ASoundManager::Cue::HasEventOfType( SOUNDEVENT a_eEventType ) const
{
	T2_FOREACH( *oEventList.GetNodeValue(), it )
	{
		if ( it->eEventType == a_eEventType )
			return TTRUE;
	}

	return TFALSE;
}

void ASoundManager::SoundEventManager::SetEventHandler( SOUNDEVENT a_eEventType, EventHandler::Callback_t a_fnHandler )
{
	TASSERT( a_eEventType >= 0 && a_eEventType < SOUNDEVENT_NUMOF );
	m_aEventHandlers[ a_eEventType ].Unused     = 0;
	m_aEventHandlers[ a_eEventType ].fnCallback = a_fnHandler;
}

void ASoundManager::SoundEventManager::ExecuteEvent( SOUNDEVENT a_eEventType, ASoundManager* a_pSoundManager, SoundEvent* a_pEvent )
{
	TASSERT( a_eEventType >= 0 && a_eEventType < SOUNDEVENT_NUMOF );

	EventHandler::Callback_t fnEventCallback = m_aEventHandlers[ a_eEventType ].fnCallback;

	TASSERT( fnEventCallback != TNULL );

	// Execute event callback
	( a_pSoundManager->*fnEventCallback )( a_pEvent );
}
