#pragma once
#include <TGUI/TGUISoundFactory.h>

//-----------------------------------------------------------------------------
// Purpose: sounds of the TGUI screens of AGUISystem (see AGUISystem::CreateTGUI).
//
// NOTE: Barnyard route added on top: the GUI sounds are Barnyard's frontend
// menu sounds (the ones AGUI2Menu plays), the IDs are sound bank IDs.
//-----------------------------------------------------------------------------
class AGUISoundFactory : public Toshi::TGUISoundFactory
{
public:
	TDECLARE_CLASS( AGUISoundFactory, Toshi::TGUISoundFactory );

public:
	AGUISoundFactory();
	~AGUISoundFactory();

	//-----------------------------------------------------------------------------
	// Toshi::TGUISoundFactory
	//-----------------------------------------------------------------------------
	virtual TINT16 GetSoundID( const PPropertyName& a_rcName ) override;
	virtual void   Unknown_0x18() override {}
	virtual void   Unknown_0x1C() override {}
};
