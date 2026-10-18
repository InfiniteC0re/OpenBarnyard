#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/TString8.h>
#include <Toshi/TVersion.h>
#include <ToshiTools/T2CommandLine.h>

class AImGui;

//-----------------------------------------------------------------------------
// Base class of every mod. A mod DLL exports these functions:
//
//   extern "C"
//   {
//       MOD_EXPORT AModInstance* CreateModInstance( const Toshi::T2CommandLine* a_pCommandLine );
//       MOD_EXPORT const TCHAR*  GetModName();
//       MOD_EXPORT TUINT32       GetModVersion();
//   }
//
// Only CreateModInstance is required.
//
// Game events (AGUI2 render, new game, ...) are in AModEventList.h.
//-----------------------------------------------------------------------------
#define MOD_EXPORT __declspec( dllexport )

class AModInstance : public Toshi::T2DList<AModInstance>::Node
{
public:
	friend class AModLoader;

public:
	AModInstance()          = default;
	virtual ~AModInstance() = default;

	//-----------------------------------------------------------------------------
	// Lifetime
	//-----------------------------------------------------------------------------

	// Called right after the mod is created. Returning TFALSE unloads the mod.
	virtual TBOOL OnLoad() = 0;

	// Called before the mod is deleted
	virtual void OnUnload() {}

	// Called once every mod has been loaded
	virtual void OnAllModsLoaded() {}

	// Called every frame
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) { return TTRUE; }

	//-----------------------------------------------------------------------------
	// ImGui overlay
	//-----------------------------------------------------------------------------

	// Whether the mod has a tab in the settings window
	virtual TBOOL HasSettingsUI() { return TFALSE; }

	// Renders the contents of the mod's settings tab
	virtual void OnImGuiRender( AImGui* a_pImGui ) {}

	// Whether the mod wants to draw an overlay while the settings window is closed
	virtual TBOOL IsOverlayVisible() { return TFALSE; }

	// Renders the overlay, called when IsOverlayVisible returns TTRUE
	virtual void OnImGuiRenderOverlay( AImGui* a_pImGui ) {}

	//-----------------------------------------------------------------------------

	const TCHAR*    GetName() const { return m_strName.GetString(); }
	Toshi::TVersion GetVersion() const { return m_uiVersion; }

private:
	Toshi::TString8 m_strName;
	Toshi::TVersion m_uiVersion;
};

using t_CreateModInstance = AModInstance* (*)( const Toshi::T2CommandLine* a_pCommandLine );
using t_GetModName        = const TCHAR* (*)();
using t_GetModVersion     = TUINT32 ( * )();
