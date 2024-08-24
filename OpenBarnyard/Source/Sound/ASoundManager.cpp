#include "pch.h"
#include "ASoundManager.h"
#include "ASound.h"
#include "ASoundAdvanced.h"
#include "Cameras/ACameraManager.h"
#include "Tasks/ARootTask.h"
#include "Assets/AAssetLoader.h"
#include "Memory/AMemory.h"
#include "AWaveBankFMODFSB.h"
#include "AWaveBankFMODFSBStream.h"
#include "ALoadScreen.h"
#include "ARandom.h"

#include <Plugins/PPropertyParser/PBProperties.h>

#include <fmod/fmod.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASoundManager );

#define MAX_NUM_SOUND_EVENTS 256

ASoundManager::ASoundManager() :
    m_SoundEventPool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_SOUND_EVENTS ),
    m_ChannelRefT1Pool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_SOUND_EVENTS, CHANNEL_REF_T1_SIZE ),
    m_ChannelRefT2Pool( AMemory::GetAllocator( AMemory::POOL_Sound ), MAX_NUM_SOUND_EVENTS, CHANNEL_REF_T2_SIZE ),
    m_CategoryIndices( AMemory::GetAllocator( AMemory::POOL_Sound ) ),
    m_SoundIdToSoundAdv( AMemory::GetAllocator( AMemory::POOL_Sound ) ),
    m_SoundIdToSound( AMemory::GetAllocator( AMemory::POOL_Sound ) )
{
	TIMPLEMENT();
	m_iLastAvailableSoundExSlot = -1;
	m_fCurrentTime              = 0.0f;

	TUtil::MemClear( &m_aEventHandlers, sizeof( m_aEventHandlers ) );

	m_bMuted                  = TFALSE;
	m_bUseMinHardwareChannels = TTRUE;
	m_iMinHWChannels          = 32;
	m_iNumChannels            = 32;
	m_iGlobalFrequency        = 22050;

	ms_pFileSystem = TFileManager::GetSingleton()->FindFileSystem( "local" );

	m_aEventHandlers[ SOUNDEVENT_PlayAudio ].Unused     = 0;
	m_aEventHandlers[ SOUNDEVENT_PlayAudio ].fnCallback = &ASoundManager::EventHandler_PlaySound;
}

ASoundManager::~ASoundManager()
{
	TIMPLEMENT();

	m_FreeListS2.Clear();

	FSOUND_Close();
	m_PauseListener.Disconnect();
	Reset();
}

TBOOL ASoundManager::Reset()
{
	TIMPLEMENT();
	return TTRUE;
}

TBOOL ASoundManager::OnCreate()
{
	Initialise();
	m_PauseListener.Connect(
	    g_oSystemManager.GetPauseEmitter(),
	    this,
	    []( ASoundManager* a_pSndMngr, TSystemManager* a_pSysMngr, TBOOL* a_pPaused ) {
		    a_pSndMngr->PauseAllSound( *a_pPaused );
		    return TTRUE;
	    },
	    0 );

	m_pS4 = new S4[ 32 ];

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

TBOOL ASoundManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();
	m_fCurrentTime += a_fDeltaTime;

	if ( !g_oSystemManager.IsPaused() )
	{
		if ( ARootTask::GetSingleton()->IsPaused() && ms_iPaused <= 0 )
		{
			PauseAllSound( TTRUE );
		}
		else if ( !ARootTask::GetSingleton()->IsPaused() && ms_iPaused > 0 )
		{
			PauseAllSound( TFALSE );
		}
	}

	TTODO( "FUN_005db310" );

	// Run the queued events
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

			TASSERT( pEarliestEvent->eEventType < SOUNDEVENT_NUMOF );
			EventHandler::Callback_t fnEventCallback = m_aEventHandlers[ pEarliestEvent->eEventType ].fnCallback;

			TASSERT( fnEventCallback != TNULL );

			// Execute event callback
			( this->*fnEventCallback )( pEarliestEvent );

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

	// ...

	// Update sound attributes based on the current camera position
	// TODO: replace TTRUE with some flag from the AQuestManager
	if ( ARootTask::GetSingleton()->IsGameSystemCreated() && TTRUE )
	{
		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		// Get the position
		m_oCameraData.Position = pCamera->GetMatrix().AsBasisVector4( 3 );

		// Get the forward vector
		m_oCameraData.Forward   = pCamera->GetMatrix().AsBasisVector4( 2 );
		m_oCameraData.Forward.w = 1.0f;

		// Get the up vector
		m_oCameraData.Up   = pCamera->GetMatrix().AsBasisVector4( 1 );
		m_oCameraData.Up.w = 1.0f;

		// Invert up axis as it needs to be
		m_oCameraData.Up.x = -m_oCameraData.Up.x;
		m_oCameraData.Up.y = -m_oCameraData.Up.y;
		m_oCameraData.Up.z = -m_oCameraData.Up.z;

		TFLOAT aPosition[ 3 ];
		aPosition[ 0 ] = m_oCameraData.Position.x;
		aPosition[ 1 ] = m_oCameraData.Position.y;
		aPosition[ 2 ] = m_oCameraData.Position.z;

		TFLOAT aVelocity[ 3 ];
		aVelocity[ 0 ] = m_oCameraData.Velocity.x;
		aVelocity[ 1 ] = m_oCameraData.Velocity.y;
		aVelocity[ 2 ] = m_oCameraData.Velocity.z;

		FSOUND_3D_Listener_SetAttributes( aPosition, aVelocity, m_oCameraData.Forward.x, m_oCameraData.Forward.y, -m_oCameraData.Forward.z, m_oCameraData.Up.x, m_oCameraData.Up.y, -m_oCameraData.Up.z );
	}

	FSOUND_Update();
	return TTRUE;
}

void ASoundManager::OnDestroy()
{
	m_UnkList1.Clear();
	m_FreeListS4.Clear();
	delete[] m_pS4;
	m_PauseListener.Disconnect();
}

TINT ASoundManager::PlaySound( ASoundWaveId a_iSound )
{
	return PlaySoundEx( a_iSound, 1.0f, TFALSE, 0.0f, -1 );
}

TINT ASoundManager::PlaySoundEx( ASoundWaveId a_iSound, TFLOAT a_fVolume, TBOOL a_bFlag, TFLOAT a_fStartDelay, TINT a_iTrack )
{
	if ( a_iSound == ASOUNDWAVEID_INVALID )
		return -1;

	// Look if the sound is stored to be played
	auto pFoundNode = m_SoundIdToSoundAdv.FindNode( a_iSound );

	if ( pFoundNode == m_SoundIdToSoundAdv.End() )
		return -1;

	// The sound is found, can use it
	ASoundAdvanced* pSound = pFoundNode->GetValue()->GetSecond();

	// Check if no waves are stored in the sound
	if ( pSound->m_vecWaves.Size() == 0 )
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
		iFlags |= 1;
		pCue->fStartTime2 = m_fCurrentTime;
	}

	// Setup basic settings on the cue
	pCue->fVolume        = ( a_fVolume > 1.0f ) ? 1.0f : a_fVolume;
	pCue->pSoundAdvanced = pSound;
	pCue->fFrequency     = 1.0f;
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
					iFlags |= 2;
				else
					iFlags &= ~2;

				CreatePlaySoundEvent(
				    pCue,
				    iTrack,
				    iFirstWaveIndex,
				    pSound->m_vecTracks[ iTrack ] + iFirstWaveIndex,
				    iFlags,
				    a_fStartDelay,
				    -1.0f );

				iFirstWaveIndex += pSound->m_vecTracks[ iTrack ];
			}
		}
	}
	else
	{
		// Play only the specified track

		TINT iFirstWaveIndex = pSound->GetFirstWaveForTrack( a_iTrack );

		if ( pSound->m_TrackLoop.GetBit( a_iTrack ) != 0 )
			iFlags |= 2;
		else
			iFlags &= ~2;

		CreatePlaySoundEvent(
		    pCue,
		    a_iTrack,
		    iFirstWaveIndex,
		    pSound->m_vecTracks[ a_iTrack ] + iFirstWaveIndex,
		    iFlags,
		    a_fStartDelay,
		    -1.0f );
	}

	pCue->bUsed = TTRUE;

	return iCueIndex;
}

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

AWaveBank* ASoundManager::FindWaveBank( const TPString8& a_rcName )
{
	auto pFoundNode = ms_WaveBanks.FindNode( a_rcName );

	return ( pFoundNode != ms_WaveBanks.End() ) ? pFoundNode->GetValue()->GetSecond() : TNULL;
}

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
	auto pExistingWaveBank   = ms_WaveBanks.FindNode( strWaveWankBankName );
	auto pWaveBankVersionVal = pBankProperties->GetOptionalProperty( "version" );

	// Store wavebank library
	auto      pWaveBankLibraryVal = pBankProperties->GetOptionalProperty( "library" );
	TPString8 strWaveBankLibrary  = ( pWaveBankLibraryVal ) ? pWaveBankLibraryVal->GetTPString8() : TNULL;

	// Store wavebank type
	auto      pWaveBankTypeVal = pBankProperties->GetOptionalProperty( "type" );
	TPString8 strWaveBankType  = ( pWaveBankTypeVal ) ? pWaveBankTypeVal->GetTPString8() : TNULL;

	// Store wavebank path
	auto      pWaveBankPathVal = pBankProperties->GetOptionalProperty( "path" );
	TPString8 strWaveBankPath  = ( pWaveBankPathVal ) ? pWaveBankPathVal->GetTPString8() : TNULL;

	// Store wavebank extension
	auto      pWaveBankExtensionVal = pBankProperties->GetOptionalProperty( "extension" );
	TPString8 strWaveBankExtension  = ( pWaveBankExtensionVal ) ? pWaveBankExtensionVal->GetTPString8() : TNULL;

	// Create the actual wavebank from the parameters
	AWaveBank* pWaveBank = AllocateWaveBank( strWaveWankBankName, strWaveBankLibrary, strWaveBankType, strWaveBankPath );

	pWaveBank->ParseWavesData( pBankProperties, a_uiForcedFlags );

	return pWaveBank;
}

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

TPSTRING8_DECLARE( default );

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
			pSoundBank->m_pSounds = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASound[ iNumSounds ];
		}
		else
		{
			pSoundBank->m_pSoundsEx = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) ASoundAdvanced[ iNumSounds ];
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
				ASound* pSound = &pSoundBank->m_pSounds[ iSoundIndex++ ];

				pSound->m_iId = iSoundId;
				m_SoundIdToSound.Insert( iSoundId, pSound );

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
				ASoundAdvanced* pSoundEx = &pSoundBank->m_pSoundsEx[ iSoundIndex++ ];

				pSoundEx->m_iId = iSoundId;
				m_SoundIdToSoundAdv.Insert( iSoundId, pSoundEx );

				// Get category
				TPString8 strCategory;
				if ( bNoErrors && pSoundProperties->GetOptionalPropertyValue( strCategory, "category" ) )
				{
					// Find category and store it's index
					auto pFoundCategory = m_CategoryIndices.FindNode(
					    ( strCategory.GetPooledString() || strCategory.GetString8().Length() == 0 ) ? TPS8( default ) : strCategory );

					TASSERT( pFoundCategory != m_CategoryIndices.End() );
					pSoundEx->m_uiCategoryIndex = pFoundCategory->GetValue()->GetSecond();

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
							pSoundEx->m_vecWaves.Reserve( pBanks->GetSize() );

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
									ASoundAdvanced::Wave oWave;
									oWave.pWaveBank = pWaveBank;
									oWave.iId       = pWaves->GetValue( i )->GetInteger();

									pSoundEx->m_vecWaves.PushBack( oWave );

									if ( pSoundEx->m_iFlags & 4 )
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
									TFLOAT fVolume                    = pVolumes->GetValue( i )->GetFloat();
									pSoundEx->m_vecWaves[ i ].fVolume = fVolume;
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

									pSoundEx->m_vecWaves[ i / 2 ].fMinVolumeMultiplier = fVolMin;
									pSoundEx->m_vecWaves[ i / 2 ].fMaxVolumeMultiplier = fVolMax;
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
									TFLOAT fPitch                    = pPitches->GetValue( i )->GetFloat();
									pSoundEx->m_vecWaves[ i ].fPitch = fPitch;
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

									pSoundEx->m_vecWaves[ i / 2 ].fMinPitch = fPitchMin;
									pSoundEx->m_vecWaves[ i / 2 ].fMaxPitch = fPitchMax;
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
									TFLOAT fStart                    = pStarts->GetValue( i )->GetFloat();
									pSoundEx->m_vecWaves[ i ].fStart = fStart;
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
									TFLOAT fVarDelay                    = pVarDelays->GetValue( i )->GetFloat();
									pSoundEx->m_vecWaves[ i ].fVarDelay = fVarDelay;
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
									TFLOAT fWeight                    = pVarDelays->GetValue( i )->GetFloat();
									pSoundEx->m_vecWaves[ i ].iWeight = TMath::Round( fWeight );
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
									if ( pVarDelays->GetValue( i )->GetRaw() != 0 )
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

TBOOL ASoundManager::Initialise()
{
	FSOUND_Init( 44100, m_iMinHWChannels + m_iNumChannels, 0 );

	TINT num2DC, num3DC, numDC;
	FSOUND_GetNumHWChannels( &num2DC, &num3DC, &numDC );
	FSOUND_GetMaxChannels();
	FSOUND_Close();

	TBOOL bRes = FSOUND_SetMinHardwareChannels( m_bUseMinHardwareChannels ? m_iMinHWChannels : 0 );
	FSOUND_Init( 44100, m_iNumChannels, 0 );
	FSOUND_GetNumHWChannels( &num2DC, &num3DC, &numDC );
	FSOUND_GetMaxChannels();
	FSOUND_Stream_SetBufferSize( 2000 );
	FSOUND_SetBufferSize( 100 );
	FSOUND_3D_SetDistanceFactor( 1.0f );
	FSOUND_3D_SetRolloffFactor( 1.0f );
	FSOUND_3D_SetDopplerFactor( 1.0f );

	return bRes;
}

void ASoundManager::PauseAllSound( TBOOL a_bPaused )
{
	TIMPLEMENT();
}

TBOOL ASoundManager::LoadWaveBanksInfo( const TCHAR* a_szFileName )
{
	TBOOL bOpened = AAssetLoader::Load(
	    "Data/Assets/lib_wavebank.trb",
	    AAssetType_WaveBank,
	    TTRUE );

	if ( !bOpened ) return TFALSE;

	const PBProperties* pProperties =
	    AAssetLoader::CastSymbol<const PBProperties>( a_szFileName, PBProperties::TRB_SECTION_NAME, AAssetType_WaveBank );
	TVALIDPTR( pProperties );

	auto pWaveBanksVal = pProperties->GetOptionalProperty( "Wavebanks" );
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

	auto pCategoriesVal = pProperties->GetOptionalProperty( "Categories" );
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

ASoundBank* ASoundManager::FindSoundBank( const Toshi::TPString8& a_rcName )
{
	T2_FOREACH( m_SoundBanks, it )
	{
		if ( it->m_strName == a_rcName )
			return it;
	}

	return TNULL;
}

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

void ASoundManager::LoadSoundBankSamples( const Toshi::TPString8& a_rcName )
{
	if ( ASoundBank* pSoundBank = FindSoundBank( a_rcName ) )
	{
		pSoundBank->Load();
	}
}

void ASoundManager::CreatePlaySoundEvent( Cue* a_pCue, TINT a_iTrackIndex, TINT a_iFirstWaveIndex, TINT a_iLastWaveIndex, TUINT a_uiFlags, TFLOAT a_fDelay1, TFLOAT a_fDelay2 )
{
	TVALIDPTR( a_pCue );

	ASoundAdvanced* pSound = a_pCue->pSoundAdvanced;
	TVALIDPTR( pSound );

	TUINT uiFlags = a_uiFlags;

	TINT iLoopStart = pSound->m_vecLoopStarts[ a_iTrackIndex ];
	if ( iLoopStart > 0 || iLoopStart == -1 )
	{
		uiFlags &= ~2;
	}

	TRandom* pRandom = &ARandom::GetSingleton()->m_oRandom;

	if ( !HASANYFLAG( uiFlags, 8 ) )
	{
		TASSERT( a_iFirstWaveIndex <= a_iLastWaveIndex );

		if ( pSound->m_vecTracks[ a_iTrackIndex ] < 2 )
		{
			TINT iNumLeftWaves = a_iLastWaveIndex - a_iFirstWaveIndex;

			for ( TINT i = a_iFirstWaveIndex; iNumLeftWaves > 0; iNumLeftWaves-- )
			{
				ASoundAdvanced::Wave* pWave = &pSound->m_vecWaves[ i ];

				TFLOAT fVolumeMultiplier = pRandom->GetFloatMinMax( pWave->fMinVolumeMultiplier, pWave->fMaxVolumeMultiplier );
				TFLOAT fVolume           = fVolumeMultiplier * pWave->fVolume;
				TMath::Clip( fVolume, 0.0f, 1.0f );

				EventParameters oParams;
				oParams[ 0 ] = fVolume;
				oParams[ 1 ] = pRandom->GetFloatMinMax( pWave->fMinPitch, pWave->fMaxPitch );
				oParams[ 2 ] = 0.0f;

				TFLOAT fStartDelay = pWave->fStart + pRandom->GetFloat() * pWave->fVarDelay;
				if ( a_fDelay1 != -1.0f ) fStartDelay += a_fDelay1;
				if ( a_fDelay2 != -1.0f ) fStartDelay += a_fDelay2;

				SoundEvent* pEvent = CreateSoundEvent(
				    ( uiFlags & 16 ) ? SOUNDEVENT_PlayAudio : SOUNDEVENT_PlayStream,
				    fStartDelay,
				    a_pCue,
				    pWave,
				    oParams,
				    TNULL,
				    uiFlags,
				    a_iTrackIndex );

				TVALIDPTR( pEvent );
			}
		}
	}
	else
	{
		TASSERT( !"Not implemented" );
	}
}

ASoundManager::SoundEvent* ASoundManager::CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, ChannelRef* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex )
{
	TVALIDPTR( a_pCue );
	TASSERT( m_SoundEventPool.CanAllocate() );

	if ( m_SoundEventPool.CanAllocate() )
	{
		SoundEvent* pEvent = m_SoundEventPool.NewObject( a_eEventType, m_fCurrentTime + a_fDelay, a_pCue, a_pWave, a_pPlayingSound, a_uiFlags, a_iTrackIndex );
		TVALIDPTR( pEvent );

		AddEventToCue( a_pCue, pEvent );
		return pEvent;
	}

	return TNULL;
}

ASoundManager::SoundEvent* ASoundManager::CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, TFLOAT a_fCustomParam1, ChannelRef* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex )
{
	TVALIDPTR( a_pCue );
	TASSERT( m_SoundEventPool.CanAllocate() );

	if ( m_SoundEventPool.CanAllocate() )
	{
		SoundEvent* pEvent = m_SoundEventPool.NewObject( a_eEventType, m_fCurrentTime + a_fDelay, a_pCue, a_pWave, a_fCustomParam1, a_pPlayingSound, a_uiFlags, a_iTrackIndex );
		TVALIDPTR( pEvent );

		AddEventToCue( a_pCue, pEvent );
		return pEvent;
	}

	return TNULL;
}

ASoundManager::SoundEvent* ASoundManager::CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, const EventParameters& a_rcCustomParams, ChannelRef* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex )
{
	TVALIDPTR( a_pCue );
	TASSERT( m_SoundEventPool.CanAllocate() );

	if ( m_SoundEventPool.CanAllocate() )
	{
		SoundEvent* pEvent = m_SoundEventPool.NewObject( a_eEventType, m_fCurrentTime + a_fDelay, a_pCue, a_pWave, a_rcCustomParams, a_pPlayingSound, a_uiFlags, a_iTrackIndex );
		TVALIDPTR( pEvent );

		AddEventToCue( a_pCue, pEvent );
		return pEvent;
	}

	return TNULL;
}

void ASoundManager::AddEventToCue( Cue* a_pCue, SoundEvent* a_pSoundEvent )
{
	TVALIDPTR( a_pCue );
	TVALIDPTR( a_pSoundEvent );

	SoundEvent* pFirstEvent = ( !a_pCue->oEventList->IsEmpty() ) ? a_pCue->oEventList->Begin() : TNULL;
	a_pCue->oEventList->Push( a_pSoundEvent );

	// If first event in the list changed, reinsert the list in m_QueuedEventLists
	// to make sure the ordering is right. It's important because the sorting method
	// is taking the first event into account when sorting the lists
	if ( pFirstEvent != a_pCue->oEventList->Begin() )
	{
		m_QueuedSortedEventLists.ReInsert( a_pCue->oEventList );
	}
}

void ASoundManager::EventHandler_PlaySound( SoundEvent* a_pEvent )
{
	TVALIDPTR( a_pEvent );
	TASSERT( TTRUE == m_ChannelRefT1Pool.CanAllocate() );

	Cue*            pCue   = a_pEvent->pCue;
	ASoundAdvanced* pSound = pCue->pSoundAdvanced;

	if ( pSound && m_ChannelRefT1Pool.CanAllocate() )
	{
		// Allocate new ChannelRef object from the pool
		ChannelRef* pChannelRef = m_ChannelRefT1Pool.NewObject();

		// Push the channelref to the cue
		pCue->iNumChannelRefs += 1;
		pCue->oChannelRefsT1.PushBack( pChannelRef );

		TFLOAT fCategoryVolume = m_aCategories[ pCue->pSoundAdvanced->m_uiCategoryIndex ].fVolumeMultiplier;

		pChannelRef->types.t1.pWave   = a_pEvent->pWave;
		pChannelRef->types.t1.fVolume = fCategoryVolume * pCue->fVolume * a_pEvent->oParameters[ 0 ];
		TMath::Clip( pChannelRef->types.t1.fVolume, 0.0f, 1.0f );

		ASoundAdvanced::Wave* pWave        = pChannelRef->types.t1.pWave;
		pChannelRef->types.t1.uiEventFlags = a_pEvent->uiFlags;
		pChannelRef->types.t1.iFrequency   = TINT( pWave->iFrequency * a_pEvent->oParameters[ 1 ] );

		FSOUND_SAMPLE* pSample = (FSOUND_SAMPLE*)pWave->pWaveBank->GetWaveSample( pWave->iId );

		TINT iSampleMode = FSOUND_Sample_GetMode( pSample );

		if ( pChannelRef->types.t1.uiEventFlags & 2 )
			iSampleMode = iSampleMode & 0xfffffffa | 2;
		else
			iSampleMode = iSampleMode & 0xfffffff9 | 1;

		FSOUND_Sample_SetMode( pSample, iSampleMode );

		TINT iChannelHandle                      = FSOUND_PlaySoundEx( FSOUND_FREE, pSample, TNULL, TTRUE );
		pChannelRef->types.t1.iFMODChannelHandle = iChannelHandle;

		if ( iChannelHandle != -1 )
		{
			// No errors
			TINT iVolume = ( m_bMuted ) ? 0 : TINT( pChannelRef->types.t1.fVolume * 255.0f );

			FSOUND_SetVolume( iChannelHandle, iVolume );
			FSOUND_SetFrequency( iChannelHandle, pChannelRef->types.t1.iFrequency );
			FSOUND_SetPriority( iChannelHandle, TINT( pSound->m_ui8Priority ) );

			if ( pChannelRef->types.t1.uiEventFlags & 4 )
			{
				TFLOAT aPos[ 3 ];
				aPos[ 0 ] = ( pCue->vecPosition ).x;
				aPos[ 1 ] = ( pCue->vecPosition ).y;
				aPos[ 2 ] = ( pCue->vecPosition ).z;

				FSOUND_3D_SetMinMaxDistance( iChannelHandle, pSound->m_fMinDist, 10000.0f );
				FSOUND_3D_SetAttributes( iChannelHandle, aPos, TNULL );
			}
			else
			{
				FSOUND_SetPan( iChannelHandle, 128 );
			}

			if ( !HASANYFLAG( pChannelRef->types.t1.uiEventFlags, 1 ) )
			{
				FSOUND_CD_SetPaused( iChannelHandle, TFALSE );
			}
			else if ( !pCue->oEventList->IsEmpty() )
			{
				pCue->oEventList.Remove();
			}

			pSound->m_iFlags |= 32;
			pChannelRef->types.t1.uiEventFlags |= 32;

			ASoundManager* pSoundManager = ASoundManager::GetSingleton();

			T2_FOREACH( pCue->aChannelRefsT2[ 0 ], it )
			{
				it->types.t2.fnCallback(
				    0,
				    ( TREINTERPRETCAST( TUINTPTR, this ) - TREINTERPRETCAST( TUINTPTR, pSoundManager ) - TOFFSETOF( ASoundManager, m_aCues ) ) / sizeof( Cue ),
				    it->types.t2.pUserData );
			}

			TINT iTrackIndex = a_pEvent->iTrackIndex;

			if ( iTrackIndex != -1 )
			{
				TINT iLoopStart = pCue->vecLoopStarts[ iTrackIndex ];

				if ( iLoopStart == -1 )
				{
					TINT iFirstWave = pSound->GetFirstWaveForTrack( iTrackIndex );

					// Remove channel refs which has finished playing
					T2_FOREACH( pCue->oChannelRefsT1, it )
					{
						if ( !FSOUND_IsPlaying( it->types.t1.iFMODChannelHandle ) )
						{
							// NOTE: Seems to be a bug that presents in the original game.
							// Erase returns next node after removed one so it will be skipped
							// since the code just overrides current iterator
							it = pCue->oChannelRefsT1.Erase( it );
							m_ChannelRefT1Pool.FreeObject( it );
						}
					}

					CreatePlaySoundEvent(
					    pCue,
					    iTrackIndex,
					    iFirstWave,
					    pSound->m_vecTracks[ iTrackIndex ] + iFirstWave,
					    a_pEvent->uiFlags,
					    0.0f,
					    pChannelRef->types.t1.pWave->pWaveBank->GetWave( pChannelRef->types.t1.pWave->iId )->fLength );
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
					    a_pEvent->uiFlags,
					    0.0f,
					    pChannelRef->types.t1.pWave->pWaveBank->GetWave( pChannelRef->types.t1.pWave->iId )->fLength );
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

ASoundManager::Cue::Cue()
{
	bUsed          = TFALSE;
	pSoundAdvanced = TNULL;
	fStartTime     = 0.0f;
	fStartTime2    = 0.0f;
	vecPosition    = TVector4::VEC_ZERO;
	fVolume        = 1.0f;
	fFrequency     = 1.0f;
}

ASoundManager::Cue::~Cue()
{
}

void ASoundManager::Cue::Reset()
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();

	T2_FOREACH( aChannelRefsT2[ 1 ], it )
	{
		it->types.t2.fnCallback(
		    1,
		    ( TREINTERPRETCAST( TUINTPTR, this ) - TREINTERPRETCAST( TUINTPTR, pSoundManager ) - TOFFSETOF( ASoundManager, m_aCues ) ) / sizeof( Cue ),
		    it->types.t2.pUserData );
	}

	bUsed          = TFALSE;
	fStartTime     = -1.0f;
	fStartTime2    = -1.0f;
	pSoundAdvanced = TNULL;

	// Reset waves used by the channel refs
	T2_FOREACH( oChannelRefsT1, channelRef )
	{
		channelRef->types.t1.pWave->pWaveBank->ResetWave( channelRef->types.t1.pWave->iId );
	}

	// Delete allocated sound events
	while ( !oEventList->IsEmpty() )
	{
		SoundEvent* pSoundEvent = oEventList->PopFront();
		pSoundManager->m_SoundEventPool.FreeObject( pSoundEvent );
	}

	// NOTE: could this be a bug in the original?
	if ( !oEventList->IsEmpty() )
		oEventList.Remove();

	// Delete all channel refs of Type2
	for ( TINT i = 0; i < TARRAYSIZE( aChannelRefsT2 ); i++ )
	{
		while ( !aChannelRefsT2[ i ].IsEmpty() )
		{
			ChannelRef* pChannelRef = aChannelRefsT2[ i ].PopFront();
			pSoundManager->m_ChannelRefT2Pool.DeleteObject( pChannelRef );
		}
	}

	iNumChannelRefs = 0;

	while ( !oChannelRefsT1.IsEmpty() )
	{
		ChannelRef* pChannelRef = oChannelRefsT1.PopFront();
		pSoundManager->m_ChannelRefT1Pool.DeleteObject( pChannelRef );
	}

	vecLoopStarts.Clear();

	if ( IsLinked() )
		Remove();
}

ASoundManager::SoundEventList::SoundEventList()
{
	m_pEventList = new ListContainer;
}

ASoundManager::SoundEventList::~SoundEventList()
{
	if ( m_pEventList )
		delete m_pEventList;
}

ASoundManager::SoundEvent::SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, ChannelRef* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex ) :
    pWave( a_pWave ),
    eEventType( a_eEventType ),
    pPlayingSound( a_pPlayingSound ),
    uiFlags( a_uiFlags ),
    pCue( a_pCue ),
    fStartTime( a_fStartTime ),
    iTrackIndex( a_iTrackIndex )
{
}

ASoundManager::SoundEvent::SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, TFLOAT a_fCustomParam1, ChannelRef* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex ) :
    pWave( a_pWave ),
    eEventType( a_eEventType ),
    pPlayingSound( a_pPlayingSound ),
    uiFlags( a_uiFlags ),
    pCue( a_pCue ),
    fStartTime( a_fStartTime ),
    iTrackIndex( a_iTrackIndex )
{
	oParameters[ 0 ] = a_fCustomParam1;
}

ASoundManager::SoundEvent::SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, const EventParameters& a_rcCustomParams, ChannelRef* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex ) :
    pWave( a_pWave ),
    eEventType( a_eEventType ),
    pPlayingSound( a_pPlayingSound ),
    uiFlags( a_uiFlags ),
    pCue( a_pCue ),
    fStartTime( a_fStartTime ),
    iTrackIndex( a_iTrackIndex ),
    oParameters( a_rcCustomParams )
{
}
