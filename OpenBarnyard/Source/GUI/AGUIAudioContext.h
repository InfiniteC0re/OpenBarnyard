#pragma once
#include <TGUI/TGUIAudioContext.h>

//-----------------------------------------------------------------------------
// Purpose: plays the sounds of the TGUI screens of AGUISystem.
//
// NOTE: Barnyard route added on top: the IDs are sound bank IDs (see
// AGUISoundFactory), played by ASoundManager.
//-----------------------------------------------------------------------------
class AGUIAudioContext : public Toshi::TGUIAudioContext
{
public:
	TDECLARE_CLASS( AGUIAudioContext, Toshi::TGUIAudioContext );

public:
	AGUIAudioContext();
	~AGUIAudioContext();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIAudioContext
	//-----------------------------------------------------------------------------
	virtual void PlaySound( TINT16 a_iSoundID ) override;
};
