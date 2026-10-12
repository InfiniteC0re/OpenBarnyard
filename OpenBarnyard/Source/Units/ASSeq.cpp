#include "pch.h"
#include "ASSeq.h"
#include "GameInterface/ACreditsState.h"
#include "GUI/ASubtitleManager.h"
#include "ASSeqController.h"
#include "FX/AParticleEffectManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"
#include "FX/AGameTimeFXManager.h"
#include "Helpers/AGameTimeManager.h"
#include "Helpers/AWeatherManager.h"
#include "Terrain/ATerrainInterface.h"
#include "GameInterface/AGameStateController.h"

#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>
#include <File/TFile.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00627390, CreateObject 00627af0, CreateObjectInPlace 00627b50
TDEFINE_CLASS( ASSeq );

const TINT ASSeq::ms_aInstructionSizes[ INSTRUCTION_NUMOF ] = {
	0x1c, // INSTRUCTION_ACTOREVENT0
	0x14, // INSTRUCTION_ACTOREVENT1
	0x1c, // INSTRUCTION_ACTOREVENT2
	0x14, // INSTRUCTION_WAIT
	0x1c, // INSTRUCTION_ANIMATION
	0x18, // INSTRUCTION_VOICE
	0x18, // INSTRUCTION_UNK6
	0x14, // INSTRUCTION_FADE
	0x10, // INSTRUCTION_ACTORFLAG
	0x10, // INSTRUCTION_SETFLAG
	0x0c, // INSTRUCTION_PAUSE
	0x0c, // INSTRUCTION_RESUME
	0x1c, // INSTRUCTION_COMMAND
	0x00, // INSTRUCTION_END
};

void ( ASSeq::*const ASSeq::ms_afnStartInstruction[ INSTRUCTION_NUMOF ] )( TINT ) = {
	&ASSeq::StartActorEvent, // INSTRUCTION_ACTOREVENT0
	&ASSeq::StartActorEvent, // INSTRUCTION_ACTOREVENT1
	&ASSeq::StartActorEvent, // INSTRUCTION_ACTOREVENT2
	&ASSeq::StartWait,       // INSTRUCTION_WAIT
	&ASSeq::StartActorEvent, // INSTRUCTION_ANIMATION
	&ASSeq::StartVoice,      // INSTRUCTION_VOICE
	&ASSeq::StartNothing,    // INSTRUCTION_UNK6
	&ASSeq::StartFade,       // INSTRUCTION_FADE
	&ASSeq::StartActorFlag,  // INSTRUCTION_ACTORFLAG
	&ASSeq::StartSetFlag,    // INSTRUCTION_SETFLAG
	&ASSeq::StartNothing,    // INSTRUCTION_PAUSE
	&ASSeq::StartNothing,    // INSTRUCTION_RESUME
	&ASSeq::StartCommand,    // INSTRUCTION_COMMAND
	&ASSeq::StartEnd,        // INSTRUCTION_END
};

TBOOL ( ASSeq::*const ASSeq::ms_afnUpdateInstruction[ INSTRUCTION_NUMOF ] )
( TINT, TFLOAT ) = {
	&ASSeq::UpdateActorEvent,  // INSTRUCTION_ACTOREVENT0
	&ASSeq::UpdateActorEvent,  // INSTRUCTION_ACTOREVENT1
	&ASSeq::UpdateActorEvent,  // INSTRUCTION_ACTOREVENT2
	&ASSeq::UpdateWait,        // INSTRUCTION_WAIT
	&ASSeq::UpdateActorEvent,  // INSTRUCTION_ANIMATION
	&ASSeq::UpdateVoice,       // INSTRUCTION_VOICE
	&ASSeq::UpdateReturnTrue,  // INSTRUCTION_UNK6
	&ASSeq::UpdateFade,        // INSTRUCTION_FADE
	&ASSeq::UpdateReturnTrue,  // INSTRUCTION_ACTORFLAG
	&ASSeq::UpdateSetFlag,     // INSTRUCTION_SETFLAG
	&ASSeq::UpdateReturnFalse, // INSTRUCTION_PAUSE
	&ASSeq::UpdateResume,      // INSTRUCTION_RESUME
	&ASSeq::UpdateCommand,     // INSTRUCTION_COMMAND
	&ASSeq::UpdateReturnFalse, // INSTRUCTION_END
};

TINT   ASSeq::ms_iVOSubStream  = -1;
TINT   ASSeq::ms_iVOStreamTime = 0;
TFLOAT ASSeq::ms_fGameTime     = 0.0f;
TFLOAT ASSeq::ms_fVOTime       = 0.0f;
TFLOAT ASSeq::ms_fVODrift      = 0.0f;
TFLOAT ASSeq::ms_fSyncTime     = 0.0f;
TFLOAT ASSeq::ms_fSyncSpeed    = 0.2f; // NOTE: 0xffffffff in retail, Start always sets it

// $Barnyard: FUNCTION 006273a0
ASSeq::ASSeq()
    : m_szFileName( TNULL )
    , m_pData( TNULL )
    , m_pScript( TNULL )
    , m_pScriptHeader( TNULL )
    , m_bPlaying( TFALSE )
    , m_oFlags( MAX_NUM_FLAGS )
    , m_pController( TNULL )
    , m_bStarted( TFALSE )
    , m_bUnk( TFALSE )
    , m_fTime( 0.0f )
    , m_fnOnFinished( TNULL )
    , m_pOnFinishedUserData( TNULL )
    , m_vecPosition( TVector4::VEC_ZERO )
    , m_fFadeTime( 0.0f )
    , m_oUI( TFALSE, TFALSE )
    , m_bWaitingForVO( TFALSE )
    , m_bHasTransform( TFALSE )
    , m_oTransform( TMatrix44::IDENTITY )
{
	TUtil::MemClear( m_apParticles, sizeof( m_apParticles ) );
}

// $Barnyard: FUNCTION 00627480
ASSeq::ASSeq( ASSeqController* a_pController )
    : m_szFileName( TNULL )
    , m_pData( TNULL )
    , m_pScript( TNULL )
    , m_pScriptHeader( TNULL )
    , m_bPlaying( TFALSE )
    , m_oFlags( MAX_NUM_FLAGS )
    , m_pController( a_pController )
    , m_bStarted( TFALSE )
    , m_bUnk( TFALSE )
    , m_fTime( 0.0f )
    , m_fnOnFinished( TNULL )
    , m_pOnFinishedUserData( TNULL )
    , m_vecPosition( TVector4::VEC_ZERO )
    , m_oUI( TFALSE, TFALSE )
    , m_pSubtitle( TNULL )
    , m_bWaitingForVO( TFALSE )
    , m_bHasTransform( TFALSE )
    , m_oTransform( TMatrix44::IDENTITY )
{
	TUtil::MemClear( m_apParticles, sizeof( m_apParticles ) );
}

// $Barnyard: FUNCTION 006286f0
// $Barnyard: FUNCTION 00627b90
ASSeq::~ASSeq()
{
	if ( ATerrainInterface::IsSingletonCreated() )
		ATerrainInterface::GetSingleton()->SetLightOverride( TFALSE );

	StopParticles();

	DestroySubtitle();

	if ( m_szFileName )
		TFree( m_szFileName );

	if ( m_pData )
	{
		TFree( m_pData );
		m_pData = TNULL;
		TFree( m_ppTracks );
		TFree( m_pTrackTimes );
	}

	g_oSystemManager.GetScheduler()->SetDeltaTimeCalculator( TNULL );
}

// $Barnyard: FUNCTION 00627c70
TBOOL ASSeq::Load( const TCHAR* a_szFileName, const TVector4& a_rcPosition )
{
	if ( m_pData )
		return TFALSE;

	if ( !a_szFileName )
		return TTRUE;

	TString8 strFilePath( "Data/ScriptedSequence/" );
	strFilePath += a_szFileName;

	TFile* pFile = TFile::Create( strFilePath, TFILEMODE_READ );

	if ( !pFile )
		return TFALSE;

	TSIZE uiFileSize = pFile->GetSize();

	// NOTE: retail allocates four times the size of the file
	m_pData = TSTATICCAST( TUINT8, TMalloc( uiFileSize * 4 ) );

	if ( pFile->Read( m_pData, uiFileSize ) != uiFileSize )
	{
		TFree( m_pData );
		m_pData = TNULL;
	}

	pFile->Destroy();

	m_szFileName = TSTATICCAST( TCHAR, TMalloc( TStringManager::String8Length( a_szFileName ) + 1 ) );
	TStringManager::String8Copy( m_szFileName, a_szFileName );

	RelocateStrings();
	RelocatePointers();

	m_pScript       = TREINTERPRETCAST( Script*, m_pData + sizeof( FileHeader ) );
	m_pScriptHeader = m_pScript;

	m_ppTracks = TSTATICCAST( Instruction*, TMalloc( m_pScriptHeader->iNumTracks * sizeof( Instruction* ) ) );

	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
		m_ppTracks[ i ] = TNULL;

	m_pTrackTimes = TSTATICCAST( TFLOAT, TMalloc( m_pScriptHeader->iNumTracks * sizeof( TFLOAT ) ) );
	m_vecPosition = a_rcPosition;

	return TTRUE;
}

// $Barnyard: FUNCTION 00627570
TBOOL ASSeq::RelocateStrings()
{
	FileHeader* pHeader    = TREINTERPRETCAST( FileHeader*, m_pData );
	TUINT8*     pBase      = m_pData + sizeof( FileHeader );
	TINT        iNumRelocs = pHeader->uiStringRelocsSize / 4;
	TUINT32*    pRelocs    = TREINTERPRETCAST( TUINT32*, pBase + pHeader->uiStringRelocsOffset );

	for ( TINT i = 0; i < iNumRelocs; i++ )
		*TREINTERPRETCAST( TUINTPTR*, pBase + pRelocs[ i ] ) += TREINTERPRETCAST( TUINTPTR, pBase + pHeader->uiStringsOffset );

	return TTRUE;
}

// $Barnyard: FUNCTION 006275b0
TBOOL ASSeq::RelocatePointers()
{
	FileHeader* pHeader    = TREINTERPRETCAST( FileHeader*, m_pData );
	TUINT8*     pBase      = m_pData + sizeof( FileHeader );
	TINT        iNumRelocs = pHeader->uiPointerRelocsSize / 4;
	TUINT32*    pRelocs    = TREINTERPRETCAST( TUINT32*, pBase + pHeader->uiPointerRelocsOffset );

	for ( TINT i = 0; i < iNumRelocs; i++ )
		*TREINTERPRETCAST( TUINTPTR*, pBase + pRelocs[ i ] ) += TREINTERPRETCAST( TUINTPTR, pBase + pHeader->uiPointersOffset );

	return TTRUE;
}

// $Barnyard: FUNCTION 006275f0
void ASSeq::ResetTracks()
{
	if ( m_pScriptHeader )
	{
		for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
		{
			m_ppTracks[ i ] = TREINTERPRETCAST( Instruction*, m_pScript );
			m_ppTracks[ i ] = TREINTERPRETCAST( Instruction*, TREINTERPRETCAST( TUINTPTR, m_ppTracks[ i ] ) + m_pScriptHeader->pTrackOffsets[ i ] );
		}
	}

	m_pController->ClearActorFlags();
}

// $Barnyard: FUNCTION 00627640
void ASSeq::DeactivateActors()
{
	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
	{
		ASSeqActorInterface* pActor = m_pController->GetActor( m_pScriptHeader->pTrackActors[ i ] );

		if ( pActor )
			pActor->SetActive( TFALSE, this, m_pScriptHeader->pTrackActors[ i ] );
	}
}

// $Barnyard: FUNCTION 00627e50
void ASSeq::ActivateActors()
{
	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
	{
		TINT                 iActor = m_pScriptHeader->pTrackActors[ i ];
		ASSeqActorInterface* pActor = m_pController->GetActor( iActor );

		if ( pActor )
		{
			m_pController->UpdateActorFlags( iActor, TTRUE );
			pActor->SetActive( TTRUE, this, iActor );
		}
	}
}

// $Barnyard: FUNCTION 006276a0
void ASSeq::StartActorEvent( TINT a_iTrack )
{
	Instruction*         pInstruction = GetTrackInstruction( a_iTrack );
	ASSeqActorInterface* pActor       = m_pController->GetActor( GetTrackActor( a_iTrack ) );

	pActor->OnSequenceEvent( pInstruction->eType, pInstruction );
}

// $Barnyard: FUNCTION 006276d0
void ASSeq::StartWait( TINT a_iTrack )
{
	// The waits of the sequence's own track follow the voice-over
	if ( GetTrackActor( a_iTrack ) == 0 )
		m_fVOWaitTime += TSTATICCAST( WaitInstruction, GetTrackInstruction( a_iTrack ) )->fDuration;
}

// $Barnyard: FUNCTION 00627700
void ASSeq::StartFade( TINT a_iTrack )
{
	if ( m_pController->IsStoppingAllScripts() )
		return;

	FadeInstruction* pInstruction = TSTATICCAST( FadeInstruction, GetTrackInstruction( a_iTrack ) );

	if ( pInstruction->bFadeOut )
		m_pController->GetFade()->FadeOut( pInstruction->fDuration );
	else
		m_pController->GetFade()->FadeIn( pInstruction->fDuration );
}

// $Barnyard: FUNCTION 00627740
void ASSeq::StartActorFlag( TINT a_iTrack )
{
	m_pController->SetActorFlag( GetTrackActor( a_iTrack ), TSTATICCAST( ActorFlagInstruction, GetTrackInstruction( a_iTrack ) )->iActorFlag );
}

// $Barnyard: FUNCTION 00627770
void ASSeq::StartSetFlag( TINT a_iTrack )
{
	if ( GetTrackActor( a_iTrack ) == 0 )
		m_bSetFlagWaited = TFALSE;
}

// $Barnyard: FUNCTION 00627790
void ASSeq::StartEnd( TINT a_iTrack )
{
	m_iNumFinishedTracks++;
}

// $Barnyard: FUNCTION 006277a0
TBOOL ASSeq::UpdateActorEvent( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	Instruction*         pInstruction = GetTrackInstruction( a_iTrack );
	ASSeqActorInterface* pActor       = m_pController->GetActor( GetTrackActor( a_iTrack ) );

	return pActor->UpdateEvent( pInstruction->eType, pInstruction, a_fDeltaTime );
}

// $Barnyard: FUNCTION 006277e0
TBOOL ASSeq::UpdateWait( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	WaitInstruction* pInstruction = TSTATICCAST( WaitInstruction, GetTrackInstruction( a_iTrack ) );

	if ( pInstruction->iWaitFlag != 0 )
	{
		if ( m_oFlags.IsSet( pInstruction->iWaitFlag ) )
			return TTRUE;
	}
	else if ( GetTrackActor( a_iTrack ) != 0 || ms_iVOSubStream == -1 )
	{
		if ( m_pTrackTimes[ a_iTrack ] >= pInstruction->fDuration )
			return TTRUE;
	}
	else if ( ms_fSyncTime >= m_fVOWaitTime )
	{
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00627870
TBOOL ASSeq::UpdateFade( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	return m_pController->GetFade()->IsOver();
}

// $Barnyard: FUNCTION 00627890
TBOOL ASSeq::UpdateSetFlag( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	m_oFlags.Set( GetTrackInstruction( a_iTrack )->iFlag );

	if ( GetTrackActor( a_iTrack ) != 0 )
		return TTRUE;

	// The sequence's own track restarts the voice-over timing and waits a frame
	m_fVOWaitTime = 0.0f;
	ms_fSyncTime  = 0.0f;

	if ( m_bSetFlagWaited )
		return TTRUE;

	m_bSetFlagWaited = TTRUE;
	return TFALSE;
}

// $Barnyard: FUNCTION 00627920
TBOOL ASSeq::UpdateResume( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
	{
		TINT         iActor       = m_pScriptHeader->pTrackActors[ i ];
		Instruction* pInstruction = m_ppTracks[ i ];

		// Every track but the sequence's own and the voice-over's
		if ( iActor == 0 || iActor == 15 )
			continue;

		ASSeqActorInterface* pActor = m_pController->GetActor( iActor );

		if ( pActor )
			pActor->StopSequenceAnimation();

		// Skip everything up to the next pause
		while ( TTRUE )
		{
			if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONSTART )
				m_oFlags.Set( pInstruction->iFlag );

			if ( pInstruction->eType == INSTRUCTION_PAUSE )
				break;

			if ( pInstruction->eType == INSTRUCTION_END )
				goto NEXT_TRACK;

			if ( pInstruction->eType == INSTRUCTION_SETFLAG || pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONEND )
				m_oFlags.Set( pInstruction->iFlag );

			pInstruction = NextInstruction( pInstruction );
		}

		if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONEND )
			m_oFlags.Set( pInstruction->iFlag );

		m_ppTracks[ i ]    = NextInstruction( pInstruction );
		m_pTrackTimes[ i ] = 0.0f;
		StartTrackInstruction( i );

	NEXT_TRACK:;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00627ab0
AParticleEffectHandle** __stdcall ASSeq::OnParticleFinished( AParticleEffectHandle* a_pHandle, AParticleEffectHandle** a_ppSlot )
{
	*a_ppSlot = TNULL;
	return a_ppSlot;
}

// $Barnyard: FUNCTION 00627ac0
void ASSeq::StopParticles()
{
	for ( TINT i = 0; i < MAX_NUM_PARTICLES; i++ )
	{
		if ( m_apParticles[ i ] )
		{
			m_apParticles[ i ]->GetInstance()->Kill();
			m_apParticles[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 00627ea0
void ASSeq::OnFinished()
{
	if ( ATerrainInterface::IsSingletonCreated() )
		ATerrainInterface::GetSingleton()->SetLightOverride( TFALSE );

	StopParticles();
	DeactivateActors();
	m_pController->ClearActorFlags();

	if ( m_fFadeTime > 0.0f )
		m_pController->GetFade()->FadeIn( m_fFadeTime );

	if ( m_fnOnFinished )
		m_fnOnFinished( this, m_pOnFinishedUserData );

	DestroySubtitle();

	AVOStream::GetSingleton()->Stop();

	ms_iVOSubStream = -1;
	m_bWaitingForVO = TFALSE;
	g_oSystemManager.GetScheduler()->SetDeltaTimeCalculator( TNULL );
	ms_fSyncSpeed = 0.2f;
}

// $Barnyard: FUNCTION 00627f50
void ASSeq::StartVoice( TINT a_iTrack )
{
	VoiceInstruction* pInstruction = TSTATICCAST( VoiceInstruction, GetTrackInstruction( a_iTrack ) );

	if ( pInstruction->iSoundType == VoiceInstruction::SOUNDTYPE_CUE )
		ASoundManager::GetSingleton()->PlayCue( ASoundId( pInstruction->iSound ) );
}

// $Barnyard: FUNCTION 00627f80
TBOOL ASSeq::UpdateCommand( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	CommandInstruction* pInstruction = TSTATICCAST( CommandInstruction, GetTrackInstruction( a_iTrack ) );

	if ( TStringManager::String8Compare( pInstruction->szCommand, "ShowCredits" ) == 0 )
	{
		// Wait until the credits are over
		if ( AGameStateController::GetSingleton()->GetCurrentState()->GetClass()->IsA( &TGetClass( ACreditsState ) ) )
			return TTRUE;
	}
	else if ( TStringManager::String8Compare( pInstruction->szCommand, "particle" ) == 0 )
	{
		const TCHAR*    szEffect       = pInstruction->szArgument1;
		AModelInstance* pModelInstance = m_pController->GetAttachmentManager().GetModelInstance();

		// NOTE: AModelInstance::GetBoneID( const TCHAR* ) (00610e40) inlined
		TINT iBone   = pModelInstance->GetSkeleton()->GetBoneID( pInstruction->szArgument2, 0 );
		TINT iEffect = AParticleEffectManager::FindEffect( szEffect );

		TINT iSlot;

		for ( iSlot = 0; iSlot < MAX_NUM_PARTICLES; iSlot++ )
		{
			if ( !m_apParticles[ iSlot ] )
				break;
		}

		if ( iSlot >= MAX_NUM_PARTICLES )
			iSlot = -1;

		if ( iEffect < 0 || iBone < 0 || iSlot < 0 )
			return TTRUE;

		TVector4 vecPosition  = TVector4::VEC_ZERO;
		TVector4 vecDirection = TVector4::VEC_POSZ;

		AParticleEffectHandle* pHandle = AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, vecPosition, vecDirection );

		if ( !pHandle )
			return TTRUE;

		pHandle->AttachToBone( pModelInstance, iBone, TTRUE, TTRUE );

		m_apParticles[ iSlot ]         = pHandle;
		pHandle->m_pOnFinishedUserData = &m_apParticles[ iSlot ];
		pHandle->m_fnOnFinished        = TREINTERPRETCAST( AParticleEffectHandle::t_OnFinished, &OnParticleFinished );

		return TTRUE;
	}

	TINT                 iActor = GetTrackActor( a_iTrack );
	ASSeqActorInterface* pActor = m_pController->GetActor( iActor );

	if ( pActor )
		return pActor->UpdateEvent( pInstruction->eType, pInstruction, a_fDeltaTime );

	// The sequence's own track waits for the player creation screens
	return iActor != 0 || TStringManager::String8Compare( pInstruction->szCommand, "UI" ) != 0 || m_oUI.IsActive() != TTRUE;
}

// NOTE: inlined in retail (~ASSeq, OnFinished, SkipToFlag, UpdateVoice)
void ASSeq::DestroySubtitle()
{
	if ( m_pSubtitle )
	{
		ASubtitleManager::GetSingleton()->StopSubtitle( m_pSubtitle );
		m_pSubtitle = TNULL;
	}
}

// $Barnyard: FUNCTION 00628150
void ASSeq::UpdateSubtitles()
{
	AVOStream* pVOStream = AVOStream::GetSingleton();

	if ( m_pSubtitle )
	{
		// Follow the voice-over
		m_pSubtitle->SetTime( pVOStream->GetTime() );
		return;
	}

	if ( pVOStream->GetLengthMs() <= 0 )
		return;

	const TCHAR* szLineName = pVOStream->GetLineName();

	if ( !szLineName )
		return;

	// The subtitle is named after the line, lower case and without the .wav extension
	TCHAR szName[ 64 ];
	TINT  iLength = TStringManager::String8Length( szLineName );
	TStringManager::String8Copy( szName, szLineName );
	TStringManager::String8ToLowerCase( szName );

	if ( iLength > 4 && TStringManager::String8Compare( szName + iLength - 4, ".wav" ) == 0 )
		szName[ iLength - 4 ] = 0;

	m_pSubtitle = ASubtitleManager::GetSingleton()->PlaySubtitle( szName, -1.0f, TFALSE );
}

// $Barnyard: FUNCTION 00628220
TFLOAT ASSeq::CalculateDeltaTime( TFLOAT a_fDeltaTime )
{
	if ( g_oSystemManager.IsPaused() )
		return 0.0f;

	AVOStream* pVOStream = AVOStream::GetSingleton();
	TINT       iVOTime   = 0;

	if ( ms_iVOSubStream != -1 )
	{
		iVOTime = pVOStream->GetTime();

		// Wait for the voice-over to play
		if ( !pVOStream->IsPlaying() )
			return 0.0f;

		if ( iVOTime - ms_iVOStreamTime <= 0 )
		{
			// The stream time hasn't moved, estimate it
			ms_fVODrift += a_fDeltaTime;
			ms_fVOTime += a_fDeltaTime;
		}
		else
		{
			TFLOAT fVOTime = ms_fVOTime - ms_fVODrift;
			ms_fVODrift    = 0.0f;
			ms_fVOTime     = TFLOAT( iVOTime - ms_iVOStreamTime ) * 0.001f + fVOTime;
		}

		ms_iVOStreamTime = iVOTime;
		ms_fGameTime += a_fDeltaTime;
	}

	ms_fSyncTime += a_fDeltaTime;

	TFLOAT fDifference = ms_fVOTime - ms_fGameTime;
	TFLOAT fAdjustment;

	if ( fDifference > 0.0001f )
	{
		// The game is behind the voice-over
		fAdjustment = ms_fSyncSpeed * a_fDeltaTime;

		if ( fAdjustment > fDifference )
			fAdjustment = fDifference;
	}
	else if ( fDifference < -0.0001f )
	{
		// The game is ahead of the voice-over, never slow it down by more than a half
		fAdjustment = -( ms_fSyncSpeed * a_fDeltaTime );

		if ( fAdjustment < fDifference )
			fAdjustment = fDifference;

		TFLOAT fHalfDeltaTime = a_fDeltaTime * 0.5f;

		if ( a_fDeltaTime + fAdjustment < fHalfDeltaTime )
			fAdjustment = -fHalfDeltaTime;
	}
	else
	{
		fAdjustment = 0.0f;
	}

	if ( fAdjustment != 0.0f )
	{
		a_fDeltaTime += fAdjustment;
		ms_fGameTime += fAdjustment;
		ms_fSyncTime += fAdjustment;
	}

	if ( ms_iVOSubStream != -1 && pVOStream->IsPlaying() )
		ms_fGameTime = ms_fSyncTime - TFLOAT( iVOTime ) * 0.001f + ms_fGameTime;

	return a_fDeltaTime;
}

// $Barnyard: FUNCTION 00628400
void ASSeq::SkipToFlag( TINT a_iFlag )
{
	// Every actor jumps to its wait for the flag
	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
	{
		Instruction* pInstruction = m_ppTracks[ i ];
		TINT         iActor       = m_pScriptHeader->pTrackActors[ i ];

		if ( iActor == 0 )
			continue;

		ASSeqActorInterface* pActor = m_pController->GetActor( iActor );

		if ( iActor == 15 || iActor == 19 )
		{
			// The voice-over
			if ( ms_iVOSubStream != -1 )
			{
				AVOStream::GetSingleton()->Stop();
				ms_iVOSubStream = -1;
				m_bWaitingForVO = TFALSE;
			}

			DestroySubtitle();
		}
		else if ( pActor )
		{
			pActor->StopSequenceAnimation();
		}

		while ( TTRUE )
		{
			if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONSTART )
				m_oFlags.Set( pInstruction->iFlag );

			if ( pInstruction->eType == INSTRUCTION_WAIT )
			{
				if ( TSTATICCAST( WaitInstruction, pInstruction )->iWaitFlag == a_iFlag )
					break;

				if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONEND )
					m_oFlags.Set( pInstruction->iFlag );
			}
			else
			{
				if ( pInstruction->eType == INSTRUCTION_END )
					goto NEXT_ACTOR_TRACK;

				if ( pInstruction->eType == INSTRUCTION_SETFLAG || pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONEND )
					m_oFlags.Set( pInstruction->iFlag );
			}

			pInstruction = NextInstruction( pInstruction );
		}

		m_ppTracks[ i ]    = pInstruction;
		m_pTrackTimes[ i ] = 0.0f;
		StartTrackInstruction( i );

	NEXT_ACTOR_TRACK:;
	}

	// The sequence's own tracks jump to where they raise the flag, raising the ones on the way
	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
	{
		if ( m_pScriptHeader->pTrackActors[ i ] != 0 )
			continue;

		Instruction* pInstruction = m_ppTracks[ i ];

		while ( TTRUE )
		{
			if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONSTART )
				m_oFlags.Set( pInstruction->iFlag );

			if ( pInstruction->eType == INSTRUCTION_SETFLAG )
			{
				if ( pInstruction->iFlag == a_iFlag )
					break;

				m_oFlags.Set( pInstruction->iFlag );
			}
			else if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONEND )
			{
				m_oFlags.Set( pInstruction->iFlag );
			}

			pInstruction = NextInstruction( pInstruction );
		}

		m_ppTracks[ i ]    = pInstruction;
		m_pTrackTimes[ i ] = 0.0f;
		StartTrackInstruction( i );
	}
}

// $Barnyard: FUNCTION 00628710
void ASSeq::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bPlaying )
		return;

	if ( !m_bStarted )
	{
		// Wait for the screen to fade out before starting
		if ( m_pController->GetFade()->IsOver() )
		{
			m_bStarted = TTRUE;

			if ( m_fFadeTime > 0.0f )
				m_pController->GetFade()->FadeIn( m_fFadeTime );

			ActivateActors();

			for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
			{
				m_pTrackTimes[ i ] = 0.0f;
				StartTrackInstruction( i );
			}
		}

		return;
	}

	if ( m_bWaitingForVO && ms_iVOSubStream != -1 )
	{
		// Wait for the voice-over to start
		if ( !AVOStream::GetSingleton()->IsPlaying() )
			return;
		m_bWaitingForVO = TFALSE;
	}

	m_fTime += a_fDeltaTime;

	for ( TINT i = 0; i < m_pScriptHeader->iNumTracks; i++ )
	{
		while ( TTRUE )
		{
			Instruction* pInstruction = m_ppTracks[ i ];

			if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONSTART && !m_oFlags.IsSet( pInstruction->iFlag ) )
				m_oFlags.Set( pInstruction->iFlag );

			TBOOL bOver = ( this->*ms_afnUpdateInstruction[ pInstruction->eType ] )( i, a_fDeltaTime );
			m_pTrackTimes[ i ] += a_fDeltaTime;

			if ( !bOver )
				break;

			if ( pInstruction->uiFlags & INSTRUCTIONFLAGS_SETFLAG_ONEND && !m_oFlags.IsSet( pInstruction->iFlag ) )
				m_oFlags.Set( pInstruction->iFlag );

			NextTrackInstruction( i );
			m_pTrackTimes[ i ] = 0.0f;
			StartTrackInstruction( i );
		}
	}

	if ( m_iNumFinishedTracks == m_pScriptHeader->iNumTracks )
	{
		OnFinished();
		m_bPlaying = TFALSE;
	}
}

// $Barnyard: FUNCTION 00628980
TBOOL ASSeq::Start( TFLOAT a_fFadeTime, TFLOAT a_fSyncSpeed )
{
	if ( m_bPlaying )
		return TFALSE;

	ResetTracks();
	m_iNumFinishedTracks = 0;
	m_oFlags.Clear();
	m_fFadeTime = a_fFadeTime;
	m_bPlaying  = TTRUE;

	if ( a_fFadeTime > 0.0f )
		m_pController->GetFade()->FadeOut( a_fFadeTime );

	m_bStarted       = TFALSE;
	m_bUnk           = TTRUE;
	m_fTime          = 0.0f;
	m_bWaitingForVO  = TFALSE;
	m_fVOWaitTime    = 0.0f;
	m_bUnk2          = TFALSE;
	m_bSetFlagWaited = TFALSE;

	g_oSystemManager.GetScheduler()->SetDeltaTimeCalculator( CalculateDeltaTime );

	ms_iVOStreamTime = 0;
	ms_fGameTime     = 0.0f;
	ms_fVOTime       = 0.0f;
	ms_fVODrift      = 0.0f;
	ms_fSyncTime     = 0.0f;

	if ( a_fSyncSpeed <= 0.0f || a_fSyncSpeed >= 1.0f )
		ms_fSyncSpeed = 0.2f;
	else
		ms_fSyncSpeed = a_fSyncSpeed;

	return TTRUE;
}

// $Barnyard: FUNCTION 00628a80
void ASSeq::Stop()
{
	if ( m_bPlaying )
	{
		OnFinished();
		m_bPlaying = TFALSE;

		AVOStream::GetSingleton()->Stop();
		ms_iVOSubStream = -1;
		m_bWaitingForVO = TFALSE;
	}
}

// $Barnyard: FUNCTION 00628ac0
TBOOL ASSeq::UpdateVoice( TINT a_iTrack, TFLOAT a_fDeltaTime )
{
	if ( !m_bPlaying )
		return TTRUE;

	VoiceInstruction* pInstruction = TSTATICCAST( VoiceInstruction, GetTrackInstruction( a_iTrack ) );

	// Sound cues are played by StartVoice
	if ( pInstruction->iSoundType != VoiceInstruction::SOUNDTYPE_VOSTREAM )
		return TTRUE;

	AVOStream* pVOStream = AVOStream::GetSingleton();

	if ( ms_iVOSubStream != -1 )
	{
		if ( ms_iVOSubStream != pInstruction->iSound )
		{
			// Another line is still playing
			if ( pVOStream->IsPlaying() )
				pVOStream->Stop();

			ms_iVOSubStream = -1;
			m_bWaitingForVO = TFALSE;
			DestroySubtitle();
		}

		if ( ms_iVOSubStream != -1 )
		{
			// Waiting for the line to start
			if ( pVOStream->IsPlayRequested() )
				return TFALSE;

			if ( pVOStream->IsPlaying() )
			{
				UpdateSubtitles();
				return TFALSE;
			}

			// The line is over
			ms_iVOSubStream = -1;
			DestroySubtitle();
			return TTRUE;
		}
	}

	// Start the line
	pVOStream->Play( pInstruction->iSound, TFALSE );
	ms_iVOSubStream  = pInstruction->iSound;
	ms_iVOStreamTime = 0;
	DestroySubtitle();

	m_bUnk2         = TFALSE;
	m_bWaitingForVO = TTRUE;
	ms_fVOTime      = 0.0f;
	ms_fGameTime    = 0.0f;
	m_fVOWaitTime   = 0.0f;
	ms_fSyncTime    = 0.0f;

	return TFALSE;
}

// $Barnyard: FUNCTION 00628c10
void ASSeq::StartCommand( TINT a_iTrack )
{
	CommandInstruction* pInstruction = TSTATICCAST( CommandInstruction, GetTrackInstruction( a_iTrack ) );

	if ( TStringManager::String8Compare( pInstruction->szCommand, "ShowCredits" ) == 0 )
	{
		AGameStateController::GetSingleton()->PushState( new ACreditsState( TTRUE, TTRUE ) );
		m_pController->GetFade()->FadeIn( 1.0f );
		Stop();
	}
	else if ( TStringManager::String8Compare( pInstruction->szCommand, "SetDayTransition" ) == 0 )
	{
		// Argument 1: the day phase (or the name of an AGameTimeFX), argument 2: the weather
		AWeatherManager::WEATHERTYPE eWeather  = AWeatherManager::GetWeatherFromName( pInstruction->szArgument2 );
		AGameTimeManager::DAYPHASE   eDayPhase = -1;

		if ( pInstruction->szArgument1 )
			eDayPhase = AGameTimeManager::GetSingleton()->GetDayPhaseFromName( TPString8( pInstruction->szArgument1 ) );

		if ( eDayPhase != -1 )
		{
			if ( eWeather == -1 )
				eWeather = AWeatherManager::GetSingleton()->GetCurrentWeather();

			AGameTimeFXManager* pGameTimeFXManager = AGameTimeFXManager::GetSingleton();
			pGameTimeFXManager->SetForcedFX( pGameTimeFXManager->GetWeatherSettings( eWeather, eDayPhase ) );
		}
		else
		{
			if ( pInstruction->szArgument1 && pInstruction->szArgument1[ 0 ] != '\0' )
			{
				AGameTimeFXManager* pGameTimeFXManager = AGameTimeFXManager::GetSingleton();
				pGameTimeFXManager->SetForcedFX( pGameTimeFXManager->FindFX( pInstruction->szArgument1 ) );
			}

			if ( eWeather != -1 )
				AWeatherManager::GetSingleton()->SetCurrentWeather( eWeather );
		}
		return;
	}
	else if ( TStringManager::String8Compare( pInstruction->szCommand, "SetLightMag" ) == 0 )
	{
		TINT iLightMag = T2String8::StringToInt( pInstruction->szArgument1 );
		ATerrainInterface::GetSingleton()->SetLightMag( iLightMag, TStringManager::String8ToFloat( pInstruction->szArgument2 ) );
		return;
	}
	else if ( TStringManager::String8Compare( pInstruction->szCommand, "SetLightOveride" ) == 0 )
	{
		ATerrainInterface::GetSingleton()->SetLightOverride( T2String8::StringToInt( pInstruction->szArgument1 ) != 0 );
		return;
	}

	TINT                 iActor = GetTrackActor( a_iTrack );
	ASSeqActorInterface* pActor = m_pController->GetActor( iActor );

	if ( pActor )
	{
		pActor->OnSequenceEvent( pInstruction->eType, pInstruction );
		return;
	}

	// The sequence's own track opens the player creation screens
	if ( iActor == 0 && TStringManager::String8Compare( pInstruction->szCommand, "UI" ) == 0 )
		m_oUI.Show( pInstruction->szArgument1 );
}
