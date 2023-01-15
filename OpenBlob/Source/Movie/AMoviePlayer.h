#pragma once

#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Core/TEvent.h"

class AMoviePlayer : public Toshi::TSingleton<AMoviePlayer>
{

	AMoviePlayer()
	{
		m_emitter = Toshi::TGenericEmitter<AMoviePlayer>(this);
		m_bInitialised = false;
		m_bIsFullscreen = true;
		m_bIsMuted = false;
		m_bIsHidden = true;
	}

	virtual ~AMoviePlayer() = default;
	virtual void Mute(bool mute = true) { m_bIsMuted = mute; }
	virtual bool IsFullScreen() { return m_bIsFullscreen; }
	virtual void SetHideMovie(bool hide = true) { m_bIsHidden = hide; }
	virtual void SetFullScreen(bool fullscreen = true) { m_bIsFullscreen = fullscreen; }
	virtual bool OnUpdate(float param_1) { return true; }
	virtual bool OnCreate() { TASSERT(TFALSE == m_bInitialised); m_bInitialised = true; }
	virtual bool OnDestroy() { TASSERT(TTRUE == m_bInitialised); m_bInitialised = false; }
	virtual void OnActivate() { }
	virtual void OnDeactivate() { }

	Toshi::TGenericEmitter<AMoviePlayer> m_emitter; // 0x4

	bool m_bInitialised; // 0x10
	bool m_bIsFullscreen; // 0x11
	bool m_bIsMuted; // 0x12
	bool m_bIsHidden; // 0x13

};

