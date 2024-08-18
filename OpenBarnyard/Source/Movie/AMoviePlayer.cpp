#include "pch.h"
#include "AMoviePlayer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS_NORUNTIME( AMoviePlayer );

AMoviePlayer::AMoviePlayer() :
    m_SomeEmitter( this )
{
	m_bCreated      = TFALSE;
	m_bIsFullscreen = TTRUE;
	m_bIsMuted      = TFALSE;
	m_bIsHidden     = TTRUE;
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
