#pragma once
#include "ASSeqFlag.h"
#include "ASSeqUI.h"

#include <Toshi/TObject.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class ASubtitle;

class ASSeqController;
class AParticleEffectHandle;

// A scripted sequence (cut-scene), loaded from Data/ScriptedSequence/<name>.tso.
//
// The script has a track per actor (ASSeqActorInterface, looked up through ASSeqController by
// the actor id of the track). Every track is a list of instructions that run one after another;
// the tracks are played in parallel and synchronise through the flags (ASSeqFlag) that
// instructions raise and wait for. Track actor 0 is the sequence itself (its waits follow the
// voice-over), actors 15 and 19 play the voice-over.
//
// TClass 0x007b4acc (size 0x57fc, runtime creatable), vtable 0x00737d54.
class ASSeq : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ASSeq, Toshi::TObject );

	static constexpr TINT MAX_NUM_FLAGS     = 256;
	static constexpr TINT MAX_NUM_PARTICLES = 15;

	// Header of the .tso file, the offsets are relative to the end of the header
	struct FileHeader
	{
		TUINT32 uiStringRelocsOffset;  // 0x00, offsets of the string pointers
		TUINT32 uiStringRelocsSize;    // 0x04, in bytes
		TUINT32 uiStringsOffset;       // 0x08
		TUINT32 uiPointerRelocsOffset; // 0x0c, offsets of the other pointers
		TUINT32 uiPointerRelocsSize;   // 0x10, in bytes
		TUINT32 uiPointersOffset;      // 0x14
	};

	// The script, right after the file header
	struct Script
	{
		const TCHAR* szName;        // 0x00
		TINT         iUnk;          // 0x04
		TINT         iNumTracks;    // 0x08
		TINT*        pTrackActors;  // 0x0c, actor id of every track
		TUINT32*     pTrackOffsets; // 0x10, offset of the first instruction of every track
	};

	using INSTRUCTION = TINT;
	enum INSTRUCTION_ : INSTRUCTION
	{
		INSTRUCTION_ACTOREVENT0 = 0,  // passed to the actor (ASSeqCameraHelper: position offset)
		INSTRUCTION_ACTOREVENT1 = 1,  // passed to the actor
		INSTRUCTION_ACTOREVENT2 = 2,  // passed to the actor
		INSTRUCTION_WAIT        = 3,  // waits for a flag or some time
		INSTRUCTION_ANIMATION   = 4,  // passed to the actor (ASSeqActorInterface::EVENT_ANIMATION)
		INSTRUCTION_VOICE       = 5,  // plays a voice-over sub stream or a sound cue
		INSTRUCTION_UNK6        = 6,  // does nothing
		INSTRUCTION_FADE        = 7,  // fades the screen (ASSeqFade)
		INSTRUCTION_ACTORFLAG   = 8,  // sets a flag of the actor in ASSeqController
		INSTRUCTION_SETFLAG     = 9,  // raises its flag
		INSTRUCTION_PAUSE       = 10, // stops the track until an INSTRUCTION_RESUME skips it
		INSTRUCTION_RESUME      = 11, // resumes the paused tracks of the other actors
		INSTRUCTION_COMMAND     = 12, // named command (ASSeqActorInterface::EVENT_COMMAND)
		INSTRUCTION_END         = 13, // end of the track
		INSTRUCTION_NUMOF
	};

	using INSTRUCTIONFLAGS = TUINT32;
	enum INSTRUCTIONFLAGS_ : INSTRUCTIONFLAGS
	{
		INSTRUCTIONFLAGS_NONE            = 0,
		INSTRUCTIONFLAGS_SETFLAG_ONSTART = BITFLAG( 1 ), // raises iFlag when the instruction starts
		INSTRUCTIONFLAGS_SETFLAG_ONEND   = BITFLAG( 2 ), // raises iFlag when the instruction is over
	};

	// Common part of every instruction, see ms_aInstructionSizes for their sizes
	struct Instruction
	{
		INSTRUCTION      eType;   // 0x00
		INSTRUCTIONFLAGS uiFlags; // 0x04
		TINT             iFlag;   // 0x08
	};

	struct WaitInstruction : Instruction
	{
		TINT   iWaitFlag; // 0x0c, waits for this flag if not 0
		TFLOAT fDuration; // 0x10
	};

	struct VoiceInstruction : Instruction
	{
		using SOUNDTYPE = TINT;
		enum SOUNDTYPE_ : SOUNDTYPE
		{
			SOUNDTYPE_VOSTREAM = 0, // iSound is a sub stream of AVOStream
			SOUNDTYPE_CUE      = 1, // iSound is a sound cue (ASoundId)
		};

		const TCHAR* szName;     // 0x0c
		TINT         iSound;     // 0x10, sub stream or ASoundId
		SOUNDTYPE    iSoundType; // 0x14
	};

	struct FadeInstruction : Instruction
	{
		TINT   bFadeOut;  // 0x0c
		TFLOAT fDuration; // 0x10
	};

	struct ActorFlagInstruction : Instruction
	{
		TINT iActorFlag; // 0x0c
	};

	struct CommandInstruction : Instruction
	{
		const TCHAR* szCommand;   // 0x0c
		const TCHAR* szArgument1; // 0x10
		const TCHAR* szArgument2; // 0x14
		const TCHAR* szArgument3; // 0x18
	};

	using t_OnFinished = void( __stdcall* )( ASSeq* a_pSSeq, void* a_pUserData );

public:
	ASSeq();
	ASSeq( ASSeqController* a_pController );
	~ASSeq();

	// Loads the script file, a_rcPosition is the origin of the sequence
	TBOOL Load( const TCHAR* a_szFileName, const Toshi::TVector4& a_rcPosition );

	// Starts playing the sequence, fades in within a_fFadeTime if it's positive.
	// a_fSyncSpeed is how fast the game time catches up with the voice-over (0.2 if not within (0, 1))
	TBOOL Start( TFLOAT a_fFadeTime, TFLOAT a_fSyncSpeed );

	// Stops playing the sequence
	void Stop();

	void Update( TFLOAT a_fDeltaTime );

	// Jumps every track to the point where a_iFlag is used (skipping the sequence)
	void SkipToFlag( TINT a_iFlag );

	// $Barnyard: FUNCTION 00627690
	const TCHAR* GetName() const { return m_pScript->szName; }

	const TCHAR* GetFileName() const { return m_szFileName; }

	// $Barnyard: FUNCTION 00627a90
	TINT GetTrackActor( TINT a_iTrack ) const { return m_pScriptHeader->pTrackActors[ a_iTrack ]; }

	// $Barnyard: FUNCTION 00627aa0
	TINT GetNumTracks() const { return m_pScriptHeader->iNumTracks; }

	TBOOL IsPlaying() const { return m_bPlaying; }

	ASSeqUI& GetUI() { return m_oUI; }

	TBOOL                   HasTransform() const { return m_bHasTransform; }
	const Toshi::TMatrix44& GetTransform() const { return m_oTransform; }
	const Toshi::TVector4&  GetPosition() const { return m_vecPosition; }

	// Places the sequence relative to the terrain locators (used by the minigames)
	void SetTransform( const Toshi::TMatrix44& a_rcTransform )
	{
		m_oTransform    = a_rcTransform;
		m_bHasTransform = TTRUE;
	}

private:
	// Makes the string pointers of the script absolute
	TBOOL RelocateStrings();

	// Makes the other pointers of the script absolute
	TBOOL RelocatePointers();

	// Moves every track to its first instruction
	void ResetTracks();

	void ActivateActors();
	void DeactivateActors();

	// Called once every track is over
	void OnFinished();

	void StopParticles();
	void UpdateSubtitles();
	void DestroySubtitle();

	Instruction* GetTrackInstruction( TINT a_iTrack ) const { return m_ppTracks[ a_iTrack ]; }

	// Instruction handlers, see ms_afnStartInstruction and ms_afnUpdateInstruction.
	// The update handlers return TTRUE once the instruction is over.
	void  StartActorEvent( TINT a_iTrack );
	TBOOL UpdateActorEvent( TINT a_iTrack, TFLOAT a_fDeltaTime );
	void  StartWait( TINT a_iTrack );
	TBOOL UpdateWait( TINT a_iTrack, TFLOAT a_fDeltaTime );
	void  StartVoice( TINT a_iTrack );
	TBOOL UpdateVoice( TINT a_iTrack, TFLOAT a_fDeltaTime );
	void  StartFade( TINT a_iTrack );
	TBOOL UpdateFade( TINT a_iTrack, TFLOAT a_fDeltaTime );
	void  StartActorFlag( TINT a_iTrack );
	void  StartSetFlag( TINT a_iTrack );
	TBOOL UpdateSetFlag( TINT a_iTrack, TFLOAT a_fDeltaTime );
	TBOOL UpdateResume( TINT a_iTrack, TFLOAT a_fDeltaTime );
	void  StartCommand( TINT a_iTrack );
	TBOOL UpdateCommand( TINT a_iTrack, TFLOAT a_fDeltaTime );
	void  StartEnd( TINT a_iTrack );

	void  StartNothing( TINT a_iTrack ) {}                                           // DOES_NOTHING 00628f40
	TBOOL UpdateReturnTrue( TINT a_iTrack, TFLOAT a_fDeltaTime ) { return TTRUE; }   // RETURNS_TRUE 00558f80
	TBOOL UpdateReturnFalse( TINT a_iTrack, TFLOAT a_fDeltaTime ) { return TFALSE; } // RETURNS_FALSE 00556a00

	// Starts the instruction the track is at
	void StartTrackInstruction( TINT a_iTrack ) { ( this->*ms_afnStartInstruction[ GetTrackInstruction( a_iTrack )->eType ] )( a_iTrack ); }

	// Moves the track to its next instruction
	void NextTrackInstruction( TINT a_iTrack )
	{
		m_ppTracks[ a_iTrack ] = TREINTERPRETCAST( Instruction*, TREINTERPRETCAST( TUINTPTR, m_ppTracks[ a_iTrack ] ) + ms_aInstructionSizes[ m_ppTracks[ a_iTrack ]->eType ] );
	}

	static Instruction* NextInstruction( Instruction* a_pInstruction )
	{
		return TREINTERPRETCAST( Instruction*, TREINTERPRETCAST( TUINTPTR, a_pInstruction ) + ms_aInstructionSizes[ a_pInstruction->eType ] );
	}

	// Called by the particle effects when they are over
	static AParticleEffectHandle** __stdcall OnParticleFinished( AParticleEffectHandle* a_pHandle, AParticleEffectHandle** a_ppSlot );

	// Scheduler delta time calculator used while a sequence is playing: keeps the game time
	// in sync with the voice-over
	static TFLOAT CalculateDeltaTime( TFLOAT a_fDeltaTime );

private:
	static const TINT ms_aInstructionSizes[ INSTRUCTION_NUMOF ];                                 // 0x00778d1c
	static void ( ASSeq::*const ms_afnStartInstruction[ INSTRUCTION_NUMOF ] )( TINT );           // 0x00778d54
	static TBOOL ( ASSeq::*const ms_afnUpdateInstruction[ INSTRUCTION_NUMOF ] )( TINT, TFLOAT ); // 0x00778d8c

	static TINT   ms_iVOSubStream;  // 0x00778d18, playing voice-over sub stream, -1 if none
	static TINT   ms_iVOStreamTime; // 0x007b4ab4, last time of the voice-over stream (ms)
	static TFLOAT ms_fGameTime;     // 0x007b4ab8
	static TFLOAT ms_fVOTime;       // 0x007b4abc
	static TFLOAT ms_fVODrift;      // 0x007b4ac0
	static TFLOAT ms_fSyncTime;     // 0x007b4ac4, time since the last voice-over started
	static TFLOAT ms_fSyncSpeed;    // 0x007b4ac8

private:
	TCHAR*                 m_szFileName;                       // 0x04, TMalloc'd
	TUINT8*                m_pData;                            // 0x08, the file, TMalloc'd
	Script*                m_pScript;                          // 0x0c
	Instruction**          m_ppTracks;                         // 0x10, current instruction of every track, TMalloc'd
	Script*                m_pScriptHeader;                    // 0x14, same as m_pScript
	TBOOL                  m_bPlaying;                         // 0x18
	ASSeqFlag              m_oFlags;                           // 0x1c
	TINT                   m_iNumFinishedTracks;               // 0x28
	TFLOAT*                m_pTrackTimes;                      // 0x2c, time spent on the current instruction of every track, TMalloc'd
	ASSeqController*       m_pController;                      // 0x30
	TBOOL                  m_bStarted;                         // 0x34, set once the controller's fade is over
	TINT                   m_iUnk;                             // 0x38
	TBOOL                  m_bUnk;                             // 0x3c
	TFLOAT                 m_fTime;                            // 0x40
	t_OnFinished           m_fnOnFinished;                     // 0x44
	void*                  m_pOnFinishedUserData;              // 0x48
	Toshi::TVector4        m_vecPosition;                      // 0x4c
	TFLOAT                 m_fFadeTime;                        // 0x5c
	ASSeqUI                m_oUI;                              // 0x60
	ASubtitle*             m_pSubtitle;                        // 0x5770
	TBOOL                  m_bWaitingForVO;                    // 0x5774, set until the voice-over sub stream starts playing
	TFLOAT                 m_fVOWaitTime;                      // 0x5778, total duration of the waits of track actor 0
	TBOOL                  m_bUnk2;                            // 0x577c
	TBOOL                  m_bSetFlagWaited;                   // 0x577d
	TBOOL                  m_bHasTransform;                    // 0x577e
	Toshi::TMatrix44       m_oTransform;                       // 0x5780
	AParticleEffectHandle* m_apParticles[ MAX_NUM_PARTICLES ]; // 0x57c0
};

TSTATICASSERT( sizeof( ASSeq ) == 0x57fc );
