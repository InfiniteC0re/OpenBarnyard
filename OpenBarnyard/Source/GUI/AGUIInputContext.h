#pragma once
#include <TGUI/TGUIInputContext.h>

//-----------------------------------------------------------------------------
// Purpose: input context of the TGUI screens of AGUISystem: logs the actions
// fired by the widgets.
//
// NOTE: Barnyard route added on top. JPOG's AGUIInputContext shows the context
// commands in its HUD, Barnyard has no such display, so they aren't shown.
//-----------------------------------------------------------------------------
class AGUIInputContext : public Toshi::TGUIInputContext
{
public:
	TDECLARE_CLASS( AGUIInputContext, Toshi::TGUIInputContext );

public:
	AGUIInputContext();
	~AGUIInputContext();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIInputContext
	//-----------------------------------------------------------------------------
	virtual void FireWidgetAction( Toshi::TGUIWidget* a_pWidget, Toshi::TGUIWidgetAction* a_pAction ) override;
	virtual void SetContextCommand( ContextCommandInfo& a_rCommand, TINT a_iCommandID, const TWCHAR* a_wszLabel ) override {}
};
