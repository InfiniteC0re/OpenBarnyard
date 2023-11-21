#include "pch.h"
#include "AMoviePlayer.h"

AMoviePlayer::AMoviePlayer()
{
	m_bCreated = TFALSE;
	m_bIsFullscreen = TTRUE;
	m_bIsMuted = TFALSE;
	m_bIsHidden = TTRUE;
}

TBOOL AMoviePlayer::OnCreate()
{
	m_bCreated = TTRUE;
	return TTRUE;
}

void AMoviePlayer::OnDestroy()
{
	m_bCreated = TFALSE;
}
