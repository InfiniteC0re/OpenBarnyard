#pragma once
#include "AModInstance.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Loads mods from Mods\OpenBarnyard\*.dll and updates them every frame.
//
// The DLLs live in a subfolder because the Mods folder itself belongs to the
// legacy BYModCore, whose mods would crash this build. Mod data stays in
// Mods\<ModName>\, see GetModsDirectory.
//-----------------------------------------------------------------------------
class MODAPI AModLoader
    : public Toshi::TTask
    , public Toshi::TSingleton<AModLoader>
{
public:
	TDECLARE_CLASS( AModLoader, Toshi::TTask );

public:
	AModLoader();
	~AModLoader();

	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	Toshi::T2DList<AModInstance>& GetMods() { return m_oMods; }
	AModInstance*                 FindMod( const TCHAR* a_szName );

	// Folder with the mods data, ends with a slash
	static const TCHAR* GetModsDirectory();

	// Folder the mod DLLs are loaded from, ends with a slash
	static const TCHAR* GetModsBinariesDirectory();

private:
	void LoadMods();
	void LoadMod( const TCHAR* a_szPath );
	void UnloadMods();

private:
	Toshi::T2DList<AModInstance> m_oMods;
	Toshi::T2CommandLine         m_oCommandLine;
};
