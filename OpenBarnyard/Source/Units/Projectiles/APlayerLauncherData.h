#pragma once
#include "Units/AUnitStaticData.h"

// Projectile launchers of a player unit type ("APlayerLauncherData" block): which
// launchers are enabled and which one sits in the primary and secondary slot.
// Read by AUnitPlayer::Create.
class APlayerLauncherData : public AUnitStaticData
{
public:
	TDECLARE_CLASS( APlayerLauncherData, AUnitStaticData );

	// Launcher indices (bit index in m_eFlags, value of the slots)
	enum LAUNCHER : TINT
	{
		LAUNCHER_NONE = -1,
		LAUNCHER_MILK_SQUIRT,  // AMilkSquirtProjectileLauncher
		LAUNCHER_AIMED_SQUIRT, // AAimedSquirtProjectileLauncher
		LAUNCHER_NUMOF
	};

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE         = 0,
		FLAGS_MILK_SQUIRT  = BITFLAG( LAUNCHER_MILK_SQUIRT ),
		FLAGS_AIMED_SQUIRT = BITFLAG( LAUNCHER_AIMED_SQUIRT ),
	};

public:
	APlayerLauncherData();
	~APlayerLauncherData();

	virtual TBOOL Create( const PBProperties* a_pProperties ) OVERRIDE; // +0x0c

	FLAGS GetFlags() const { return m_eFlags; }
	TBOOL IsLauncherEnabled( TINT a_iLauncher ) const { return ( m_eFlags & ( 1 << a_iLauncher ) ) != 0; }
	TINT  GetPrimaryLauncher() const { return m_iPrimaryLauncher; }
	TINT  GetSecondaryLauncher() const { return m_iSecondaryLauncher; }

	// Returns the LAUNCHER of a launcher class name (AProjectileLauncher subclass), or LAUNCHER_NONE.
	static TINT GetLauncherIndex( const TCHAR* a_szClassName );

private:
	FLAGS m_eFlags;             // 0x0c, "EnabledLaunchers"
	TINT  m_iPrimaryLauncher;   // 0x10, "PrimaryLauncher", LAUNCHER_NONE unless enabled
	TINT  m_iSecondaryLauncher; // 0x14, "SecondaryLauncher", LAUNCHER_NONE unless enabled
};

TSTATICASSERT( sizeof( APlayerLauncherData ) == 0x18 );
