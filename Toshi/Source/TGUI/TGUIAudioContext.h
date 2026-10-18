#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: plays the sounds of the GUI (the sounds of the screens, see
// TGUIScreen::MoveFocus), implemented by the game.
//-----------------------------------------------------------------------------
class TGUIAudioContext : public TObject
{
public:
	TDECLARE_CLASS( TGUIAudioContext, TObject );

public:
	TGUIAudioContext();
	~TGUIAudioContext();

	// +0x14: Plays a sound (an ID from the sound factory, TGUISoundFactory::GetSoundID)
	// NOTE: the name is unknown
	virtual void PlaySound( TINT16 a_iSoundID ) = 0;
};

TOSHI_NAMESPACE_END
