#include "pch.h"
#include "AGUIAudioContext.h"
#include "Sound/ASoundManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUIAudioContext );

AGUIAudioContext::AGUIAudioContext()
{
}

AGUIAudioContext::~AGUIAudioContext()
{
}

void AGUIAudioContext::PlaySound( TINT16 a_iSoundID )
{
	if ( a_iSoundID >= 0 && ASoundManager::IsSingletonCreated() )
		ASoundManager::GetSingleton()->PlayCue( a_iSoundID );
}
