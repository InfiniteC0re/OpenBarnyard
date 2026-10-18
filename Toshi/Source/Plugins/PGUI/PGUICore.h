#pragma once
#include <Toshi/TObject.h>

namespace Toshi
{
class TGUIScreen;
}

//-----------------------------------------------------------------------------
// Purpose: the PGUI core plugin: the widgets (panels, buttons, labels, text
// boxes, sliders, dialogs, menus, list and table boxes, splitters) and the
// layout managers the [dialog] blocks of the definition files create by name.
//
// NOTE: not in the original. JPOG builds PGUICore as a DLL, its classes are
// registered when it is loaded. Here they are in the Toshi static library: the
// host calls PGUIInitialise before creating the TGUIManager so that the linker
// keeps them (TGUIManager initialises them with the other GUI classes).
//-----------------------------------------------------------------------------
void PGUIInitialise();

// The classes registered by PGUIInitialise
const Toshi::TClass* const* PGUIGetClasses( TINT& a_rNumClasses );

// Plays the sound a_szSound of a_szClass ("PGUIButton" "Down"...) of the sound factory of the GUI with the audio context
// of a_pScreen
// NOTE: the name is unknown, the original inlines it (the screen must have an audio context)
void PGUIPlaySound( Toshi::TGUIScreen* a_pScreen, const TCHAR* a_szClass, const TCHAR* a_szSound );
