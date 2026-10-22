#pragma once
#include "ASound.h"

#include <Toshi/TTask.h>
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AGameState;

//-----------------------------------------------------------------------------
// Purpose: Plays the background music.
// In the main game it's an AMultiplexStream chosen from the time of day, the
// weather, the place of the player, the bike and the quests (fading the layers
// in and out), elsewhere it's the sound cue of the current game state (GetSound)
// or m_iForcedSound.
// DAT_0078c42c is the instance.
//
// Size 0x74, created by AGameSystemManager.
//-----------------------------------------------------------------------------
class MODAPI AMusicManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AMusicManager>
{
public:
	TDECLARE_CLASS( AMusicManager, Toshi::TTask );

public:
	AMusicManager();
	~AMusicManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	// The music of the frontend
	void PlayBackgroundMusic( ASoundId a_eSoundId );
	void StopBackgroundMusic();

	// a_bEnable TFALSE stops the music (a_bCloseStream closes the stream of the main game right away),
	// TTRUE starts it again. Nothing happens while a state transition is going on unless the music is stopped.
	void SetEnabled( TBOOL a_bEnable, TBOOL a_bCloseStream );

	// The sound played instead of the sound of the current state (-1: none)
	// NOTE: inlined in retail
	void     SetForcedSound( ASoundId a_iSound ) { m_iForcedSound = a_iSound; }
	ASoundId GetForcedSound() const { return m_iForcedSound; }

	// Removes the forced sound, a_bFadeOut fades the music out
	void ClearForcedSound( TBOOL a_bFadeOut );

	// TTRUE if no music cue is playing
	TBOOL IsMusicOver();

	// Volume of the music cue (0-1)
	void   SetVolume( TFLOAT a_fVolume );
	TFLOAT GetVolume() const { return m_fVolume; }

	TBOOL                  IsNearBenGrave() const { return m_bNearBenGrave; }
	const Toshi::TVector4& GetBenGravePosition() const { return m_vecBenGravePos; }

	const Toshi::TPString8& GetCurrentStream() const { return m_strCurrentStream; }

	// Updates the music next frame even if nothing has changed
	void ForceUpdate() { m_bForceUpdate = TTRUE; }

private:
	// Plays the sound of the current state again
	void Restart();

private:
	Toshi::TVector4  m_vecBenGravePos;      // 0x20
	TBOOL            m_bNearBenGrave;       // 0x30, Ben's grave music ("BDeath")
	AGameState*      m_pLastState;          // 0x34
	TINT             m_iLastDayPhase;       // 0x38
	TINT             m_iLastNumChildStates; // 0x3c
	TBOOL            m_bEnabled;            // 0x40
	TFLOAT           m_fVolume;             // 0x44
	TINT             m_iCue;                // 0x48
	ASoundId         m_iSound;              // 0x4c
	TINT             m_iBackgroundMusicCue; // 0x50
	ASoundId         m_iForcedSound;        // 0x54
	Toshi::TPString8 m_strCurrentStream;    // 0x58, stream of the main game
	Toshi::TPString8 m_strMissionStream;    // 0x5c, random mission stream while a quest sequence plays
	TFLOAT           m_fFadeTime;           // 0x60
	TFLOAT           m_fFadeDuration;       // 0x64
	TBOOL            m_bFadingOut;          // 0x68
	TBOOL            m_bFadingIn;           // 0x69
	TBOOL            m_bForceUpdate;        // 0x6a
	TINT             m_iSavedStreamTime;    // 0x6c, time of the music while the start menu is open
	TINT             m_iRainCue;            // 0x70

#ifdef BARNYARD_LATEST
	// The Wii picks a random day/bike track each time the current one ends (Wii +0x5c/+0x60)
	Toshi::TPString8 m_strDayStream;
	Toshi::TPString8 m_strBikeStream;
#endif
};
