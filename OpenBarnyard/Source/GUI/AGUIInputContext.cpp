#include "pch.h"
#include "AGUIInputContext.h"

#include <TGUI/TGUIWidget.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUIInputContext );

AGUIInputContext::AGUIInputContext()
{
}

AGUIInputContext::~AGUIInputContext()
{
}

void AGUIInputContext::FireWidgetAction( TGUIWidget* a_pWidget, TGUIWidgetAction* a_pAction )
{
	TINFO( "[TGUI] widget action of '%s'\n", a_pWidget->GetName().GetString() );
	TGUIInputContext::FireWidgetAction( a_pWidget, a_pAction );
}
