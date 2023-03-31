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
	AMoviePlayer() : m_Emitter(this)
	{
		m_bInitialised = false;
		m_bIsFullscreen = true;
		m_bIsMuted = false;
		m_bIsHidden = true;
	}

	void ThrowEvent(AMovieEvent::Type type) { m_Emitter.Throw(&type); }

	virtual ~AMoviePlayer() = default;
	virtual bool PlayMovie(const char* fileName, uint32_t soundChannel, uint32_t flags) = 0;
	virtual void StopMovie() = 0;
	virtual void PauseMovie(bool pause) = 0;
	virtual void Mute(bool mute = true) { m_bIsMuted = mute; }
	virtual bool IsMoviePlaying() = 0;
	virtual bool IsMoviePaused() = 0;
	virtual bool IsFullScreen() { return m_bIsFullscreen; }
	virtual int Unk1() { return 0; }
	virtual int Unk2() { return 0; }
	virtual int Unk3() { return 0; }
	virtual void SetHideMovie(bool hide = true) { m_bIsHidden = hide; }
	virtual void SetFullScreen(bool fullscreen = true) { m_bIsFullscreen = fullscreen; }
	virtual void OnRender(float deltaTime) = 0;
	virtual void OnUpdate(float deltaTime) { }
	virtual void OnCreate() { TASSERT(TFALSE == m_bInitialised); m_bInitialised = true; }
	virtual void OnDestroy() { TASSERT(TTRUE == m_bInitialised); m_bInitialised = false; }
	virtual void OnActivate() { }
	virtual void OnDeactivate() { }

protected:
	Toshi::TEmitter<AMoviePlayer> m_Emitter; // 0x4
	bool m_bInitialised; // 0x10
	bool m_bIsFullscreen; // 0x11
	bool m_bIsMuted; // 0x12
	bool m_bIsHidden; // 0x13

};

