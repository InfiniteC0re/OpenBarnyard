#include "pch.h"
#include "AMusicManager.h"
#include "ASoundManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AMusicManager );

// $Barnyard: FUNCTION 005d4760
AMusicManager::AMusicManager()
{
	m_eBackgroundMusicCue = -1;
}

// $Barnyard: FUNCTION 005d46b0
void AMusicManager::PlayBackgroundMusic( ASoundId a_eSoundId )
{
	if ( m_eBackgroundMusicCue == -1 )
		m_eBackgroundMusicCue = ASoundManager::GetSingleton()->PlayCue( a_eSoundId );
}

// $Barnyard: FUNCTION 005d46d0
void AMusicManager::StopBackgroundMusic()
{
	TASSERT( m_eBackgroundMusicCue != -1 );

	ASoundManager::GetSingleton()->StopCue( m_eBackgroundMusicCue );
	m_eBackgroundMusicCue = -1;
}
