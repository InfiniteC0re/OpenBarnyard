#pragma once
#include "AGameState.h"
#include "HUD/ALetterBoxHudElement.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TString8.h>
#include <Math/TVector4.h>

class ACameraHelper;
class AModelInstance;

// Plays a scripted sequence (cut-scene) of ASSeqController: loads Data/ScriptedSequence/<script>.tso,
// binds the actors of its tracks (camera, attachment manager, actor units that it spawns if needed)
// and removes itself (or moves on) once the sequence is over.
//
// TClass 0x00781ba8 (size 0x29c), vtable 0x0071b230.
class ASSeqState : public AGameState
{
public:
	TDECLARE_CLASS( ASSeqState, AGameState );

	static constexpr TINT MAX_NUM_UNITS   = 30;
	static constexpr TINT NUM_BARN_MODELS = 4;

public:
	// a_szScriptFile:         the .tso file without its extension
	// a_szName:               name of the sequence in the script
	// a_szCamera:             model of the sequence camera (Data/Models)
	// a_iUnk:                 1 sets m_fSyncSpeed to 0.1
	// a_bRemoveWhenOver:      removes the state once the sequence is over
	// a_bDestroyUnits:        destroys the actor units spawned by the state when it's removed
	// a_bRestartMusic:        restarts the music when the state is removed
	// a_bLoadNextState:       loads the next state of the game instead of removing the state
	// a_bRestoreGameTimeFX:   restores the time of day lighting when the state is removed
	// a_bShowChapterTitle:    shows the chapter title of the story sequences
	// a_bFade:                starts with an opaque screen
	// a_bStartImmediately:    starts the sequence in OnInsertion
	ASSeqState( const TCHAR* a_szScriptFile, const TCHAR* a_szName, const TCHAR* a_szCamera, const Toshi::TVector4& a_rcPosition, TINT a_iUnk, TBOOL a_bRemoveWhenOver, TBOOL a_bDestroyUnits, TBOOL a_bRestartMusic, TBOOL a_bLoadNextState, TBOOL a_bRestoreGameTimeFX, TBOOL a_bShowChapterTitle, TBOOL a_bFade, TBOOL a_bStartImmediately );
	~ASSeqState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) override;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnInsertion() override;
	virtual void  OnRemoval() override;
	virtual void  OnActivate() override;
	virtual void  OnDeactivate() override;

	static TBOOL IsPlaying() { return ms_bIsPlaying; }

	void SetCanSkip( TBOOL a_bCanSkip ) { m_bCanSkip = a_bCanSkip; }
	void SetLoading( TBOOL a_bLoading ) { m_bLoading = a_bLoading; }

private:
	// Called once the sequence is over
	void Finish();

	// Releases the proxies of the spawned actor units
	void ReleaseUnits();

private:
	inline static TBOOL ms_bSkipped          = TFALSE; // 0x00781ba4
	inline static TBOOL ms_bFinishOnActivate = TFALSE; // 0x00781ba5
	inline static TBOOL ms_bIsPlaying        = TFALSE; // 0x007810eb

	inline static TCHAR ms_szActorTypeName[ 35 ]; // 0x00781bd8

private:
	ALetterBoxHudElement    m_oLetterBox;                      // 0x0a4
	Toshi::TString8         m_strScriptFile;                   // 0x1a8
	Toshi::TString8         m_strName;                         // 0x1b4
	Toshi::TString8         m_strCamera;                       // 0x1c0
	TBOOL                   m_bUnk1;                           // 0x1cc
	ACameraHelper*          m_pPreviousCameraHelper;           // 0x1d0
	Toshi::TVector4         m_vecPosition;                     // 0x1d4
	TFLOAT                  m_fTime;                           // 0x1e4
	TBOOL                   m_bRemoveWhenOver;                 // 0x1e8
	TBOOL                   m_bCanSkip;                        // 0x1e9
	TBOOL                   m_bDestroyUnits;                   // 0x1ea
	TBOOL                   m_bRestartMusic;                   // 0x1eb
	TBOOL                   m_bLoadNextState;                  // 0x1ec
	TBOOL                   m_bRestoreGameTimeFX;              // 0x1ed
	TINT                    m_iNumUnits;                       // 0x1f0
	Toshi::TRef<AUnitProxy> m_aUnits[ MAX_NUM_UNITS ];         // 0x1f4
	TFLOAT                  m_fStartTime;                      // 0x26c
	TFLOAT                  m_fStartDelay;                     // 0x270
	TBOOL                   m_bStarted;                        // 0x274
	TBOOL                   m_bShowChapterTitle;               // 0x275
	AModelInstance*         m_apBarnModels[ NUM_BARN_MODELS ]; // 0x278, the barn of SS10
	TBOOL                   m_bFade;                           // 0x288
	TBOOL                   m_bStartImmediately;               // 0x289
	TFLOAT                  m_fSyncSpeed;                      // 0x28c
	TINT                    m_iUnk2;                           // 0x290
	TINT                    m_iUnk3;                           // 0x294
	TBOOL                   m_bLoading;                        // 0x298, shows the loading screen in OnInsertion
};
