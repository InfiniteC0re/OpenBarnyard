#include "pch.h"
#include "AMoviePlayer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TDEFINE_CLASS_NORUNTIME( AMoviePlayer );

// $Barnyard: FUNCTION 006056f0
AMoviePlayer::AMoviePlayer()
    : m_SomeEmitter( this )
{
	m_bCreated      = TFALSE;
	m_bIsFullscreen = TTRUE;
	m_bIsMuted      = TFALSE;
	m_bIsHidden     = TTRUE;
}

// $Barnyard: FUNCTION 006056c0
TBOOL AMoviePlayer::OnCreate()
{
	m_bCreated = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 006056d0
void AMoviePlayer::OnDestroy()
{
	m_bCreated = TFALSE;
}
