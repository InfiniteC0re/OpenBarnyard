#pragma once

//-----------------------------------------------------------------------------
// -ps2glyphs / -gcglyphs: text prompts show controller button icons instead of the
// controller button names, like the PS2 build does (PS2 or GameCube icon set).
//-----------------------------------------------------------------------------
namespace AControllerButtonGlyphs
{

// Installs the hook if the game was started with -ps2glyphs or -gcglyphs
void Initialise();

} // namespace AControllerButtonGlyphs
