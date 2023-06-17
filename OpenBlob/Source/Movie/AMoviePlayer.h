#pragma once

#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/TEvent.h"

namespace AMovieEvent
{
	enum Type
	{
		Type_Finished,
		Type_Looping,
		Type_Stopped,
	};
};

class AMoviePlayer : public Toshi::TSingleton<AMoviePlayer>
{
public:
	typedef uint32_t PlayFlags;
	enum PlayFlags_
	{
		PlayFlags_None  = 0,
		PlayFlags_Loop  = BITFIELD(0),
		PlayFlags_Muted = BITFIELD(1),
	};

public:
	AMoviePlayer() : m_Emitter(this)
	{
		m_bInitialised = TFALSE;
		m_bIsFullscreen = TTRUE;
		m_bIsMuted = TFALSE;
		m_bIsHidden = TTRUE;
	}

	void ThrowEvent(AMovieEvent::Type type) { m_Emitter.Throw(&type); }

	virtual ~AMoviePlayer() = default;
	virtual TBOOL PlayMovie(const char* fileName, uint32_t soundChannel, PlayFlags flags = PlayFlags_None) = 0;
	virtual void StopMovie() = 0;
	virtual void PauseMovie(TBOOL pause) = 0;
	virtual void Mute(TBOOL mute = TTRUE) { m_bIsMuted = mute; }
	virtual TBOOL IsMoviePlaying() = 0;
	virtual TBOOL IsMoviePaused() = 0;
	virtual TBOOL IsFullScreen() { return m_bIsFullscreen; }
	virtual int Unk1() { return 0; }
	virtual int Unk2() { return 0; }
	virtual int Unk3() { return 0; }
	virtual void SetHideMovie(TBOOL hide = TTRUE) { m_bIsHidden = hide; }
	virtual void SetFullScreen(TBOOL fullscreen = TTRUE) { m_bIsFullscreen = fullscreen; }
	virtual void OnRender(float deltaTime) = 0;
	virtual void OnUpdate(float deltaTime) { }
	virtual void OnCreate() { TASSERT(TFALSE == m_bInitialised); m_bInitialised = TTRUE; }
	virtual void OnDestroy() { TASSERT(TTRUE == m_bInitialised); m_bInitialised = TFALSE; }
	virtual void OnActivate() { }
	virtual void OnDeactivate() { }

protected:
	Toshi::TEmitter<AMoviePlayer, AMovieEvent::Type> m_Emitter; // 0x4
	TBOOL m_bInitialised; // 0x10
	TBOOL m_bIsFullscreen; // 0x11
	TBOOL m_bIsMuted; // 0x12
	TBOOL m_bIsHidden; // 0x13

};

