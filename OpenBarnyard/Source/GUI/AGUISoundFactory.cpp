#include "pch.h"
#include "AGUISoundFactory.h"
#include "SoundBank/ui.h"

#include <Plugins/PPropertyParser/PPropertyName.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGUISoundFactory );

AGUISoundFactory::AGUISoundFactory()
{
}

AGUISoundFactory::~AGUISoundFactory()
{
}

// The sound of a GUI sound name (e.g. "TGUIScreen.navigate", "PGUIButton.down"), -1 if there's none
TINT16 AGUISoundFactory::GetSoundID( const PPropertyName& a_rcName )
{
	const TCHAR* szSound = a_rcName.GetSubName().GetString();

	if ( TStringManager::String8CompareNoCase( szSound, "navigate" ) == 0 )
		return TINT16( soundbank::UI_MENUCLICK );

	if ( TStringManager::String8CompareNoCase( szSound, "down" ) == 0 )
		return TINT16( soundbank::UI_MENUOK );

	return -1;
}
