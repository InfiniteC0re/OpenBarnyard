#pragma once

class PPropertyName;

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: finds the sounds of the GUI by name (e.g. "TGUIScreen.navigate"),
// implemented by the game. The screens play them with their audio context.
//
// NOTE: the names of the functions are unknown.
//-----------------------------------------------------------------------------
class TGUISoundFactory : public TObject
{
public:
	TDECLARE_CLASS( TGUISoundFactory, TObject );

public:
	TGUISoundFactory();
	~TGUISoundFactory();

	// +0x14: The ID of the sound named a_rcName
	virtual TINT16 GetSoundID( const PPropertyName& a_rcName ) = 0;

	// +0x18
	virtual void Unknown_0x18() = 0;

	// +0x1C
	virtual void Unknown_0x1C() = 0;
};

TOSHI_NAMESPACE_END
